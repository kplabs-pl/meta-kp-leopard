#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/compat.h>

#include <linux/spi/spi.h>
#include <linux/uaccess.h>

#include "spidev-leopard.h"


/*
 * This supports access to SPI devices using normal userspace I/O calls.
 * Note that while traditional UNIX/POSIX I/O semantics are half duplex,
 * and often mask message boundaries, full SPI support requires full duplex
 * transfers.  There are several kinds of internal message boundaries to
 * handle chipselect management and other protocol options.
 *
 * SPI has a character major number assigned.  We allocate minor numbers
 * dynamically using a bitmask.  You must use hotplug tools, such as udev
 * (or mdev with busybox) to create and destroy the /dev/leopard_spidevB.C device
 * nodes, since there is no fixed association of minor numbers with any
 * particular SPI bus or device.
 */
#define LEOPARD_SPIDEV_MAJOR			153	/* assigned */
#define N_SPI_MINORS			32	/* ... up to 256 */

static DECLARE_BITMAP(minors, N_SPI_MINORS);

static_assert(N_SPI_MINORS > 0 && N_SPI_MINORS <= 256);

/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for some settings can cause *lots* of trouble for other
 * devices on a shared bus:
 *
 *  - CS_HIGH ... this device will be active when it shouldn't be
 *  - 3WIRE ... when active, it won't behave as it should
 *  - NO_CS ... there will be no explicit message boundaries; this
 *	is completely incompatible with the shared bus model
 *  - READY ... transfers may proceed when they shouldn't.
 *
 * REVISIT should changing those flags be privileged?
 */
#define SPI_MODE_MASK		(SPI_MODE_X_MASK | SPI_CS_HIGH \
                | SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
                | SPI_NO_CS | SPI_READY | SPI_TX_DUAL \
                | SPI_TX_QUAD | SPI_TX_OCTAL | SPI_RX_DUAL \
                | SPI_RX_QUAD | SPI_RX_OCTAL \
                | SPI_RX_CPHA_FLIP)

struct leopard_spidev_data {
    dev_t			devt;
    spinlock_t		spi_lock;
    struct spi_device	*spi;
    struct list_head	device_entry;

    /* TX/RX buffers are NULL unless this device is open (users > 0) */
    struct mutex		buf_lock;
    unsigned		users;
    u8			*tx_buffer;
    u8			*rx_buffer;
    u32			speed_hz;
};

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 4096;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*-------------------------------------------------------------------------*/

static ssize_t
leopard_spidev_sync(struct leopard_spidev_data *leopard_spidev, struct spi_message *message)
{
    int status;
    struct spi_device *spi;

    spin_lock_irq(&leopard_spidev->spi_lock);
    spi = leopard_spidev->spi;
    spin_unlock_irq(&leopard_spidev->spi_lock);

    if (spi == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_sync(spi, message);

    if (status == 0)
        status = message->actual_length;

    return status;
}

static int leopard_spidev_message(struct leopard_spidev_data *leopard_spidev,
        struct leopard_spi_ioc_transfer *u_xfers, unsigned n_xfers)
{
    struct spi_message	msg;
    struct spi_transfer	*k_xfers;
    struct spi_transfer	*k_tmp;
    struct leopard_spi_ioc_transfer *u_tmp;
    unsigned		n, total, tx_total, rx_total;
    u8			*tx_buf, *rx_buf;
    int			status = -EFAULT;

    spi_message_init(&msg);
    k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
    if (k_xfers == NULL)
        return -ENOMEM;

    /* Construct spi_message, copying any tx data to bounce buffer.
     * We walk the array of user-provided transfers, using each one
     * to initialize a kernel version of the same transfer.
     */
    tx_buf = leopard_spidev->tx_buffer;
    rx_buf = leopard_spidev->rx_buffer;
    total = 0;
    tx_total = 0;
    rx_total = 0;
    for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
            n;
            n--, k_tmp++, u_tmp++) {
        /* Ensure that also following allocations from rx_buf/tx_buf will meet
         * DMA alignment requirements.
         */
        unsigned int len_aligned = ALIGN(u_tmp->len, ARCH_KMALLOC_MINALIGN);

        k_tmp->len = u_tmp->len;

        total += k_tmp->len;
        /* Since the function returns the total length of transfers
         * on success, restrict the total to positive int values to
         * avoid the return value looking like an error.  Also check
         * each transfer length to avoid arithmetic overflow.
         */
        if (total > INT_MAX || k_tmp->len > INT_MAX) {
            status = -EMSGSIZE;
            goto done;
        }

        if (u_tmp->rx_buf) {
            /* this transfer needs space in RX bounce buffer */
            rx_total += len_aligned;
            if (rx_total > bufsiz) {
                status = -EMSGSIZE;
                goto done;
            }
            k_tmp->rx_buf = rx_buf;
            if (copy_from_user(rx_buf, (const u8 __user *)
                        (uintptr_t) u_tmp->rx_buf,
                    u_tmp->len))
                goto done;
            rx_buf += len_aligned;
        }
        if (u_tmp->tx_buf) {
            /* this transfer needs space in TX bounce buffer */
            tx_total += len_aligned;
            if (tx_total > bufsiz) {
                status = -EMSGSIZE;
                goto done;
            }
            k_tmp->tx_buf = tx_buf;
            if (copy_from_user(tx_buf, (const u8 __user *)
                        (uintptr_t) u_tmp->tx_buf,
                    u_tmp->len))
                goto done;
            tx_buf += len_aligned;
        }

        k_tmp->cs_change = !!u_tmp->cs_change;
        k_tmp->tx_nbits = 1;
        k_tmp->rx_nbits = 1;
        k_tmp->bits_per_word = 8;
        k_tmp->delay.value = 0;
        k_tmp->delay.unit = SPI_DELAY_UNIT_USECS;
        k_tmp->speed_hz = 5000000;
        k_tmp->word_delay.value = u_tmp->word_delay_usecs;
        k_tmp->word_delay.unit = SPI_DELAY_UNIT_USECS;
        if (!k_tmp->speed_hz)
            k_tmp->speed_hz = leopard_spidev->speed_hz;
#ifdef VERBOSE
        dev_dbg(&leopard_spidev->spi->dev,
            "  xfer len %u %s%s%s%dbits %u usec %u usec %uHz\n",
            k_tmp->len,
            k_tmp->rx_buf ? "rx " : "",
            k_tmp->tx_buf ? "tx " : "",
            k_tmp->cs_change ? "cs " : "",
            k_tmp->bits_per_word ? : leopard_spidev->spi->bits_per_word,
            k_tmp->delay.value,
            k_tmp->word_delay.value,
            k_tmp->speed_hz ? : leopard_spidev->spi->max_speed_hz);
#endif
        spi_message_add_tail(k_tmp, &msg);
    }

    status = leopard_spidev_sync(leopard_spidev, &msg);
    if (status < 0)
        goto done;

    /* copy any rx data out of bounce buffer */
    for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
            n;
            n--, k_tmp++, u_tmp++) {
        if (u_tmp->rx_buf) {
            if (copy_to_user((u8 __user *)
                    (uintptr_t) u_tmp->rx_buf, k_tmp->rx_buf,
                    u_tmp->len)) {
                status = -EFAULT;
                goto done;
            }
        }
        if (u_tmp->tx_buf) {
            if (copy_to_user((u8 __user *)
                    (uintptr_t) u_tmp->tx_buf, k_tmp->tx_buf,
                    u_tmp->len)) {
                status = -EFAULT;
                goto done;
            }
        }
    }
    status = total;

done:
    kfree(k_xfers);
    return status;
}

static struct leopard_spi_ioc_transfer *
leopard_spidev_get_ioc_message(unsigned int cmd, struct leopard_spi_ioc_transfer __user *u_ioc,
        unsigned *n_ioc)
{
    u32	tmp;

    /* Check type, command number and direction */
    if (_IOC_TYPE(cmd) != LEOPARD_SPI_IOC_MAGIC
            || _IOC_NR(cmd) != _IOC_NR(LEOPARD_SPI_IOC_MESSAGE(0))
            || _IOC_DIR(cmd) != _IOC_WRITE)
        return ERR_PTR(-ENOTTY);

    tmp = _IOC_SIZE(cmd);
    if ((tmp % sizeof(struct leopard_spi_ioc_transfer)) != 0)
        return ERR_PTR(-EINVAL);
    *n_ioc = tmp / sizeof(struct leopard_spi_ioc_transfer);
    if (*n_ioc == 0)
        return NULL;

    /* copy into scratch area */
    return memdup_user(u_ioc, tmp);
}

static long
leopard_spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int			retval = 0;
    struct leopard_spidev_data	*leopard_spidev;
    struct spi_device	*spi;
    u32			tmp;
    unsigned		n_ioc;
    struct leopard_spi_ioc_transfer	*ioc;

    /* Check type and command number */
    if (_IOC_TYPE(cmd) != LEOPARD_SPI_IOC_MAGIC)
        return -ENOTTY;

    /* guard against device removal before, or while,
     * we issue this ioctl.
     */
    leopard_spidev = filp->private_data;
    spin_lock_irq(&leopard_spidev->spi_lock);
    spi = spi_dev_get(leopard_spidev->spi);
    spin_unlock_irq(&leopard_spidev->spi_lock);

    if (spi == NULL)
        return -ESHUTDOWN;

    /* use the buffer lock here for triple duty:
     *  - prevent I/O (from us) so calling spi_setup() is safe;
     *  - prevent concurrent SPI_IOC_WR_* from morphing
     *    data fields while SPI_IOC_RD_* reads them;
     *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
     */
    mutex_lock(&leopard_spidev->buf_lock);

    switch (cmd) {
    default:
        /* segmented and/or full-duplex I/O request */
        /* Check message and copy into scratch area */
        ioc = leopard_spidev_get_ioc_message(cmd,
                (struct leopard_spi_ioc_transfer __user *)arg, &n_ioc);
        if (IS_ERR(ioc)) {
            retval = PTR_ERR(ioc);
            break;
        }
        if (!ioc)
            break;	/* n_ioc is also 0 */

        /* translate to spi_message, execute */
        retval = leopard_spidev_message(leopard_spidev, ioc, n_ioc);
        kfree(ioc);
        break;
    }

    mutex_unlock(&leopard_spidev->buf_lock);
    spi_dev_put(spi);
    return retval;
}

static long
leopard_spidev_compat_ioc_message(struct file *filp, unsigned int cmd,
        unsigned long arg)
{
    struct leopard_spi_ioc_transfer __user	*u_ioc;
    int				retval = 0;
    struct leopard_spidev_data		*leopard_spidev;
    struct spi_device		*spi;
    unsigned			n_ioc, n;
    struct leopard_spi_ioc_transfer		*ioc;

    u_ioc = (struct leopard_spi_ioc_transfer __user *) compat_ptr(arg);

    /* guard against device removal before, or while,
     * we issue this ioctl.
     */
    leopard_spidev = filp->private_data;
    spin_lock_irq(&leopard_spidev->spi_lock);
    spi = spi_dev_get(leopard_spidev->spi);
    spin_unlock_irq(&leopard_spidev->spi_lock);

    if (spi == NULL)
        return -ESHUTDOWN;

    /* SPI_IOC_MESSAGE needs the buffer locked "normally" */
    mutex_lock(&leopard_spidev->buf_lock);

    /* Check message and copy into scratch area */
    ioc = leopard_spidev_get_ioc_message(cmd, u_ioc, &n_ioc);
    if (IS_ERR(ioc)) {
        retval = PTR_ERR(ioc);
        goto done;
    }
    if (!ioc)
        goto done;	/* n_ioc is also 0 */

    /* Convert buffer pointers */
    for (n = 0; n < n_ioc; n++) {
        ioc[n].rx_buf = (uintptr_t) compat_ptr(ioc[n].rx_buf);
        ioc[n].tx_buf = (uintptr_t) compat_ptr(ioc[n].tx_buf);
    }

    /* translate to spi_message, execute */
    retval = leopard_spidev_message(leopard_spidev, ioc, n_ioc);
    kfree(ioc);

done:
    mutex_unlock(&leopard_spidev->buf_lock);
    spi_dev_put(spi);
    return retval;
}

static long
leopard_spidev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    if (_IOC_TYPE(cmd) == LEOPARD_SPI_IOC_MAGIC
            && _IOC_NR(cmd) == _IOC_NR(LEOPARD_SPI_IOC_MESSAGE(0))
            && _IOC_DIR(cmd) == _IOC_WRITE)
        return leopard_spidev_compat_ioc_message(filp, cmd, arg);

    return leopard_spidev_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}

static int leopard_spidev_open(struct inode *inode, struct file *filp)
{
    struct leopard_spidev_data	*leopard_spidev = NULL, *iter;
    int			status = -ENXIO;

    mutex_lock(&device_list_lock);

    list_for_each_entry(iter, &device_list, device_entry) {
        if (iter->devt == inode->i_rdev) {
            status = 0;
            leopard_spidev = iter;
            break;
        }
    }

    if (!leopard_spidev) {
        pr_debug("leopard_spidev: nothing for minor %d\n", iminor(inode));
        goto err_find_dev;
    }

    if (!leopard_spidev->tx_buffer) {
        leopard_spidev->tx_buffer = kmalloc(bufsiz, GFP_KERNEL);
        if (!leopard_spidev->tx_buffer) {
            status = -ENOMEM;
            goto err_find_dev;
        }
    }

    if (!leopard_spidev->rx_buffer) {
        leopard_spidev->rx_buffer = kmalloc(bufsiz, GFP_KERNEL);
        if (!leopard_spidev->rx_buffer) {
            status = -ENOMEM;
            goto err_alloc_rx_buf;
        }
    }

    leopard_spidev->users++;
    filp->private_data = leopard_spidev;
    stream_open(inode, filp);

    mutex_unlock(&device_list_lock);
    return 0;

err_alloc_rx_buf:
    kfree(leopard_spidev->tx_buffer);
    leopard_spidev->tx_buffer = NULL;
err_find_dev:
    mutex_unlock(&device_list_lock);
    return status;
}

static int leopard_spidev_release(struct inode *inode, struct file *filp)
{
    struct leopard_spidev_data	*leopard_spidev;
    int			dofree;

    mutex_lock(&device_list_lock);
    leopard_spidev = filp->private_data;
    filp->private_data = NULL;

    spin_lock_irq(&leopard_spidev->spi_lock);
    /* ... after we unbound from the underlying device? */
    dofree = (leopard_spidev->spi == NULL);
    spin_unlock_irq(&leopard_spidev->spi_lock);

    /* last close? */
    leopard_spidev->users--;
    if (!leopard_spidev->users) {

        kfree(leopard_spidev->tx_buffer);
        leopard_spidev->tx_buffer = NULL;

        kfree(leopard_spidev->rx_buffer);
        leopard_spidev->rx_buffer = NULL;

        if (dofree)
            kfree(leopard_spidev);
        else
            leopard_spidev->speed_hz = leopard_spidev->spi->max_speed_hz;
    }

    if (!dofree)
        spi_slave_abort(leopard_spidev->spi);

    mutex_unlock(&device_list_lock);

    return 0;
}

static const struct file_operations leopard_spidev_fops = {
    .owner =	THIS_MODULE,
    /* REVISIT switch to aio primitives, so that userspace
     * gets more complete API coverage.  It'll simplify things
     * too, except for the locking.
     */
    .unlocked_ioctl = leopard_spidev_ioctl,
    .compat_ioctl = leopard_spidev_compat_ioctl,
    .open =		leopard_spidev_open,
    .release =	leopard_spidev_release,
    .llseek =	no_llseek,
};

/*-------------------------------------------------------------------------*/

/* The main reason to have this class is to make mdev/udev create the
 * /dev/leopard_spidevB.C character device nodes exposing our userspace API.
 * It also simplifies memory management.
 */

static struct class *leopard_spidev_class;

static const struct spi_device_id leopard_spidev_spi_ids[] = {
    { .name = "leopard-spidev" },
    {},
};
MODULE_DEVICE_TABLE(spi, leopard_spidev_spi_ids);

/*
 * leopard_spidev should never be referenced in DT without a specific compatible string,
 * it is a Linux implementation thing rather than a description of the hardware.
 */
static int leopard_spidev_of_check(struct device *dev)
{
    if (device_property_match_string(dev, "compatible", "spidev") < 0)
        return 0;

    dev_err(dev, "spidev listed directly in DT is not supported\n");
    return -EINVAL;
}

static const struct of_device_id leopard_spidev_dt_ids[] = {
    { .compatible = "kp,leopard-spidev", .data = &leopard_spidev_of_check },
    {},
};
MODULE_DEVICE_TABLE(of, leopard_spidev_dt_ids);

/* Dummy SPI devices not to be used in production systems */
static int leopard_spidev_acpi_check(struct device *dev)
{
    dev_warn(dev, "do not use this driver in production systems!\n");
    return 0;
}

static const struct acpi_device_id leopard_spidev_acpi_ids[] = {
    /*
     * The ACPI SPT000* devices are only meant for development and
     * testing. Systems used in production should have a proper ACPI
     * description of the connected peripheral and they should also use
     * a proper driver instead of poking directly to the SPI bus.
     */
    { "SPT0001", (kernel_ulong_t)&leopard_spidev_acpi_check },
    { "SPT0002", (kernel_ulong_t)&leopard_spidev_acpi_check },
    { "SPT0003", (kernel_ulong_t)&leopard_spidev_acpi_check },
    {},
};
MODULE_DEVICE_TABLE(acpi, leopard_spidev_acpi_ids);

/*-------------------------------------------------------------------------*/

static int leopard_spidev_probe(struct spi_device *spi)
{
    int (*match)(struct device *dev);
    struct leopard_spidev_data	*leopard_spidev;
    int			status;
    unsigned long		minor;
    dev_info(&spi->dev, "Leopard SPIDEV probe");

    match = device_get_match_data(&spi->dev);
    if (match) {
        status = match(&spi->dev);
        if (status)
            return status;
    }

    /* Allocate driver data */
    leopard_spidev = kzalloc(sizeof(*leopard_spidev), GFP_KERNEL);
    if (!leopard_spidev)
        return -ENOMEM;

    /* Initialize the driver data */
    leopard_spidev->spi = spi;
    spin_lock_init(&leopard_spidev->spi_lock);
    mutex_init(&leopard_spidev->buf_lock);

    INIT_LIST_HEAD(&leopard_spidev->device_entry);

    /* If we can allocate a minor number, hook up this device.
     * Reusing minors is fine so long as udev or mdev is working.
     */
    mutex_lock(&device_list_lock);
    minor = find_first_zero_bit(minors, N_SPI_MINORS);
    if (minor < N_SPI_MINORS) {
        struct device *dev;

        leopard_spidev->devt = MKDEV(LEOPARD_SPIDEV_MAJOR, minor);
        dev = device_create(leopard_spidev_class, &spi->dev, leopard_spidev->devt,
                    leopard_spidev, "leopard/spidev%d.%d",
                    spi->master->bus_num, spi_get_chipselect(spi, 0));
        status = PTR_ERR_OR_ZERO(dev);
    } else {
        dev_dbg(&spi->dev, "no minor number available!\n");
        status = -ENODEV;
    }
    if (status == 0) {
        set_bit(minor, minors);
        list_add(&leopard_spidev->device_entry, &device_list);
    }
    mutex_unlock(&device_list_lock);

    leopard_spidev->speed_hz = spi->max_speed_hz;

    if (status == 0)
        spi_set_drvdata(spi, leopard_spidev);
    else
        kfree(leopard_spidev);

    return status;
}

static void leopard_spidev_remove(struct spi_device *spi)
{
    struct leopard_spidev_data	*leopard_spidev = spi_get_drvdata(spi);

    /* prevent new opens */
    mutex_lock(&device_list_lock);
    /* make sure ops on existing fds can abort cleanly */
    spin_lock_irq(&leopard_spidev->spi_lock);
    leopard_spidev->spi = NULL;
    spin_unlock_irq(&leopard_spidev->spi_lock);

    list_del(&leopard_spidev->device_entry);
    device_destroy(leopard_spidev_class, leopard_spidev->devt);
    clear_bit(MINOR(leopard_spidev->devt), minors);
    if (leopard_spidev->users == 0)
        kfree(leopard_spidev);
    mutex_unlock(&device_list_lock);
}

static struct spi_driver leopard_spidev_spi_driver = {
    .driver = {
        .name =		"leopard_spidev",
        .of_match_table = leopard_spidev_dt_ids,
        .acpi_match_table = leopard_spidev_acpi_ids,
    },
    .probe =	leopard_spidev_probe,
    .remove =	leopard_spidev_remove,
    .id_table =	leopard_spidev_spi_ids,

    /* NOTE:  suspend/resume methods are not necessary here.
     * We don't do anything except pass the requests to/from
     * the underlying controller.  The refrigerator handles
     * most issues; the controller driver handles the rest.
     */
};

/*-------------------------------------------------------------------------*/

static int __init leopard_spidev_init(void)
{
    int status;
    /* Claim our 256 reserved device numbers.  Then register a class
     * that will key udev/mdev to add/remove /dev nodes.  Last, register
     * the driver which manages those device numbers.
     */
    status = register_chrdev(LEOPARD_SPIDEV_MAJOR, "spi", &leopard_spidev_fops);
    if (status < 0) {
        pr_err("[spidev-leopard] unable to register character device\n");
        return status;
    }

    leopard_spidev_class = class_create(THIS_MODULE, "leopard_spidev");
    if (IS_ERR(leopard_spidev_class)) {
        pr_err("[spidev-leopard] unable to create class\n");
        unregister_chrdev(LEOPARD_SPIDEV_MAJOR, leopard_spidev_spi_driver.driver.name);
        return PTR_ERR(leopard_spidev_class);
    }

    status = spi_register_driver(&leopard_spidev_spi_driver);
    if (status < 0) {
        pr_err("[spidev-leopard] unable to register driver\n");
        class_destroy(leopard_spidev_class);
        unregister_chrdev(LEOPARD_SPIDEV_MAJOR, leopard_spidev_spi_driver.driver.name);
    }

    return status;
}

static void __exit leopard_spidev_exit(void)
{
    spi_unregister_driver(&leopard_spidev_spi_driver);
    class_destroy(leopard_spidev_class);
    unregister_chrdev(LEOPARD_SPIDEV_MAJOR, leopard_spidev_spi_driver.driver.name);
}

module_init(leopard_spidev_init);
module_exit(leopard_spidev_exit);

MODULE_AUTHOR("KP Labs");
MODULE_DESCRIPTION("User mode Leopard SPI device interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:leopard-spidev");
