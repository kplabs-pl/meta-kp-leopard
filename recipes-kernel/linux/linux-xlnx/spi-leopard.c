/**
 * Leopard SPI Slave driver.
 * 
 * This driver is intended to use with spidev-leopard user space driver.
 * 
 * Upon Linux startup this driver enables RX not empty interrupt 
 * and on every occurance of that interrupt, driver will write received 
 * byte into circular rx buffer. User can later use user space 
 * SPI driver to readdata from that buffer.
 * 
 * User can write data to circular tx buffer using user space 
 * SPI driver. Upon writing tx data, driver will enable TX full interrupt, 
 * to assert irq gpio in SPI irq if SPI peripheral TX FIFO is full. 
 * This gpio shall be used by SPI master to start SPI transaction.
 * 
 * If SPI peripheral TX FIFO is not full, driver will write data 
 * from tx circular buffer to that FIFO. If driver writes
 * 128 consecutive zeroes it assumes, that all data has been sent 
 * and irq gpio is deasserted. Zeroes shall be used as packet separators.
 * 
 * Using driver from user space:
 *  #include <linux/spi/spidev-leopard.h>
 * 
 *  int fd = open("/dev/leopard/spidev1.0", O_RDWR)
 * 
 *  struct leopard_spi_ioc_transfer tr = {
 *    // size of data to write is encoded on two first bytes of tx_buffer
 *    // in little endian order
 *    .tx_buf = (unsigned long)tx_buffer,
 *    // maximum size of data to read is encoded on two first bytes
 *    // of rx_buffer in little endinan order
 *    .rx_buf = (unsigned long)rx_buffer, 
 *    // size of tx_buffer and rx_buffer which should be the same size
 *    .len = size
 *  }
 * 
 *  ioctl(fd, LEOPARD_SPI_IOC_MESSAGE(1), &tr);
 * 
 *  // after ioctl returns two first bytes of tx_buffer will contain
 *  // number of written bytes to internal tx buffer and 
 *  // two first bytes of rx_buffer will contain number of read bytes
 *  // from internal rx buffer
 * 
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>
#include <linux/circ_buf.h>

/* Name of this driver */
#define LEOPARD_SPI_NAME		"leopard-spi"

/* Register offset definitions */
#define LEOPARD_SPI_CR	0x00 /* Configuration  Register, RW */
#define LEOPARD_SPI_ISR	0x04 /* Interrupt Status Register, RO */
#define LEOPARD_SPI_IER	0x08 /* Interrupt Enable Register, WO */
#define LEOPARD_SPI_IDR	0x0c /* Interrupt Disable Register, WO */
#define LEOPARD_SPI_IMR	0x10 /* Interrupt Enabled Mask Register, RO */
#define LEOPARD_SPI_ER	0x14 /* Enable/Disable Register, RW */
#define LEOPARD_SPI_DR	0x18 /* Delay Register, RW */
#define LEOPARD_SPI_TXD	0x1C /* Data Transmit Register, WO */
#define LEOPARD_SPI_RXD	0x20 /* Data Receive Register, RO */
#define LEOPARD_SPI_SICR	0x24 /* Slave Idle Count Register, RW */
#define LEOPARD_SPI_THLD	0x28 /* Transmit FIFO Watermark Register,RW */

#define SPI_AUTOSUSPEND_TIMEOUT		3000
/*
 * SPI Configuration Register bit Masks
 *
 * This register contains various control bits that affect the operation
 * of the SPI controller
 */
#define LEOPARD_SPI_CR_MANSTRT	0x00010000 /* Manual TX Start */
#define LEOPARD_SPI_CR_CPHA		0x00000004 /* Clock Phase Control */
#define LEOPARD_SPI_CR_CPOL		0x00000002 /* Clock Polarity Control */
#define LEOPARD_SPI_CR_SSCTRL		0x00003C00 /* Slave Select Mask */
#define LEOPARD_SPI_CR_PERI_SEL	0x00000200 /* Peripheral Select Decode */
#define LEOPARD_SPI_CR_BAUD_DIV	0x00000038 /* Baud Rate Divisor Mask */
#define LEOPARD_SPI_CR_MSTREN		0x00000001 /* Master Enable Mask */
#define LEOPARD_SPI_CR_MANSTRTEN	0x00008000 /* Manual TX Enable Mask */
#define LEOPARD_SPI_CR_SSFORCE	0x00004000 /* Manual SS Enable Mask */
#define LEOPARD_SPI_CR_BAUD_DIV_4	0x00000008 /* Default Baud Div Mask */
#define LEOPARD_SPI_CR_DEFAULT	(LEOPARD_SPI_CR_MSTREN | \
                    LEOPARD_SPI_CR_SSCTRL | \
                    LEOPARD_SPI_CR_SSFORCE | \
                    LEOPARD_SPI_CR_BAUD_DIV_4)

/*
 * SPI Configuration Register - Baud rate and slave select
 *
 * These are the values used in the calculation of baud rate divisor and
 * setting the slave select.
 */

#define LEOPARD_SPI_BAUD_DIV_MAX		7 /* Baud rate divisor maximum */
#define LEOPARD_SPI_BAUD_DIV_MIN		1 /* Baud rate divisor minimum */
#define LEOPARD_SPI_BAUD_DIV_SHIFT		3 /* Baud rate divisor shift in CR */
#define LEOPARD_SPI_SS_SHIFT		10 /* Slave Select field shift in CR */
#define LEOPARD_SPI_SS0			0x1 /* Slave Select zero */
#define LEOPARD_SPI_NOSS			0xF /* No Slave select */

/*
 * SPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define LEOPARD_SPI_IXR_TXOW	0x00000004 /* SPI TX FIFO Overwater */
#define LEOPARD_SPI_IXR_MODF	0x00000002 /* SPI Mode Fault */
#define LEOPARD_SPI_IXR_RXNEMTY 0x00000010 /* SPI RX FIFO Not Empty */
#define LEOPARD_SPI_IXR_DEFAULT	(LEOPARD_SPI_IXR_TXOW | \
                    LEOPARD_SPI_IXR_MODF)
#define LEOPARD_SPI_IXR_TXFULL	0x00000008 /* SPI TX Full */
#define LEOPARD_SPI_IXR_ALL	0x0000007F /* SPI all interrupts */

/*
 * SPI Enable Register bit Masks
 *
 * This register is used to enable or disable the SPI controller
 */
#define LEOPARD_SPI_ER_ENABLE	0x00000001 /* SPI Enable Bit Mask */
#define LEOPARD_SPI_ER_DISABLE	0x0 /* SPI Disable Bit Mask */

/* Default number of chip select lines */
#define LEOPARD_SPI_DEFAULT_NUM_CS		4

#define BUFFER_SIZE 32 * 1024

/**
 * struct leopard_spi - This definition defines spi driver instance
 * @regs:		Virtual address of the SPI controller registers
 * @ref_clk:		Pointer to the peripheral clock
 * @pclk:		Pointer to the APB clock
 * @speed_hz:		Current SPI bus clock speed in Hz
 * @txbuf:		Pointer	to the TX buffer
 * @rxbuf:		Pointer to the RX buffer
 * @tx_bytes:		Number of bytes left to transfer
 * @rx_bytes:		Number of bytes requested
 * @dev_busy:		Device busy flag
 * @is_decoded_cs:	Flag for decoder property set or not
 * @tx_fifo_depth:	Depth of the TX FIFO
 */
struct leopard_spi {
    void __iomem *regs;
    struct clk *ref_clk;
    struct clk *pclk;
    unsigned int clk_rate;
    u32 speed_hz;
    struct circ_buf txbuf;
    struct circ_buf rxbuf;
    int tx_bytes;
    int rx_bytes;
    u8 dev_busy;
    u32 is_decoded_cs;
    unsigned int tx_fifo_depth;
    struct gpio_desc *irq;
    u32 zeroes;
};

/* Macros for the SPI controller read/write */
static inline u32 leopard_spi_read(struct leopard_spi *xspi, u32 offset)
{
    return readl_relaxed(xspi->regs + offset);
}

static inline void leopard_spi_write(struct leopard_spi *xspi, u32 offset, u32 val)
{
    writel_relaxed(val, xspi->regs + offset);
}

static inline bool leopard_spi_write_tx_data(struct leopard_spi *xspi, u32 val) 
{
    if ((leopard_spi_read(xspi, LEOPARD_SPI_ISR) & LEOPARD_SPI_IXR_TXFULL) != 0) {
        return false;
    }

    leopard_spi_write(xspi, LEOPARD_SPI_TXD, val);
    return true;
}

/**
 * leopard_spi_init_hw - Initialize the hardware and configure the SPI controller
 * @xspi:	Pointer to the leopard_spi structure
 * * On reset the SPI controller is configured to slave or  master mode.
 * In master mode baud rate divisor is set to 4, threshold value for TX FIFO
 * not full interrupt is set to 1 and size of the word to be transferred as 8 bit.
 *
 * This function initializes the SPI controller to disable and clear all the
 * interrupts, enable manual slave select and manual start, deselect all the
 * chip select lines, and enable the SPI controller.
 */
static void leopard_spi_init_hw(struct leopard_spi *xspi)
{
    u32 ctrl_reg = 0;

    if (xspi->is_decoded_cs)
        ctrl_reg |= LEOPARD_SPI_CR_PERI_SEL;

    leopard_spi_write(xspi, LEOPARD_SPI_ER, LEOPARD_SPI_ER_DISABLE);
    leopard_spi_write(xspi, LEOPARD_SPI_IDR, LEOPARD_SPI_IXR_ALL);

    /* Clear the RX FIFO */
    while (leopard_spi_read(xspi, LEOPARD_SPI_ISR) & LEOPARD_SPI_IXR_RXNEMTY)
        leopard_spi_read(xspi, LEOPARD_SPI_RXD);

    leopard_spi_write(xspi, LEOPARD_SPI_ISR, LEOPARD_SPI_IXR_ALL);
    leopard_spi_write(xspi, LEOPARD_SPI_CR, ctrl_reg);
    leopard_spi_write(xspi, LEOPARD_SPI_ER, LEOPARD_SPI_ER_ENABLE);
}

/**
 * leopard_spi_irq - Interrupt service routine of the SPI controller
 * @irq:	IRQ number
 * @dev_id:	Pointer to the xspi structure
 *
 * This function handles TX empty and Mode Fault interrupts only.
 * On TX empty interrupt this function reads the received data from RX FIFO and
 * fills the TX FIFO if there is any data remaining to be transferred.
 * On Mode Fault interrupt this function indicates that transfer is completed,
 * the SPI subsystem will identify the error as the remaining bytes to be
 * transferred is non-zero.
 *
 * Return:	IRQ_HANDLED when handled; IRQ_NONE otherwise.
 */
static irqreturn_t leopard_spi_irq(int irq, void *dev_id)
{
    struct spi_controller *ctlr = dev_id;
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);
    // dev_dbg(&ctlr->dev, "irq");
    irqreturn_t status;
    u32 intr_status;
    u32 intr_enabled;
    int trans_cnt;
    int head;
    int tail;
    int to_write;
    u8 data;
    bool write_status;

    status = IRQ_NONE;
    intr_status = leopard_spi_read(xspi, LEOPARD_SPI_ISR);
    intr_enabled = leopard_spi_read(xspi, LEOPARD_SPI_IMR);
    leopard_spi_write(xspi, LEOPARD_SPI_ISR, intr_status);

    if (((intr_status & LEOPARD_SPI_IXR_TXFULL) != 0) && ((intr_enabled & LEOPARD_SPI_IXR_TXFULL) != 0)) {
        gpiod_set_value(xspi->irq, 1);
        xspi->zeroes = 0;
        leopard_spi_write(xspi, LEOPARD_SPI_IDR, LEOPARD_SPI_IXR_TXFULL);
           status = IRQ_HANDLED;
    }

    if (intr_status & LEOPARD_SPI_IXR_RXNEMTY) {
        data = leopard_spi_read(xspi, LEOPARD_SPI_RXD);

        head = xspi->rxbuf.head;
        tail = READ_ONCE(xspi->rxbuf.tail);
        if (CIRC_SPACE(head, tail, BUFFER_SIZE) >= 1)  {
            xspi->rxbuf.buf[head] = data;
            smp_store_release(&xspi->rxbuf.head, (head + 1) & (BUFFER_SIZE - 1));
        }
           status = IRQ_HANDLED;
    }    

    head = smp_load_acquire(&xspi->txbuf.head);
    tail = xspi->txbuf.tail;

    to_write = CIRC_CNT(head, tail, BUFFER_SIZE);
    if (to_write > 0) {
        gpiod_set_value(xspi->irq, 1);
    }

    write_status = true;
    while (write_status == true) {
        data = (to_write > 0) ? xspi->txbuf.buf[tail] : 0;
        write_status = leopard_spi_write_tx_data(xspi, data);
        if (write_status) {
            if (to_write > 0) {
                to_write--;
                tail = (tail + 1) & (BUFFER_SIZE - 1);
                xspi->zeroes = 0;
            } else {
                if (xspi->zeroes > 128) {
                    gpiod_set_value(xspi->irq, 0);
                } else {
                    xspi->zeroes++;
                }
            }
        }
    }

    smp_store_release(&xspi->txbuf.tail, tail);
       status = IRQ_HANDLED;

    return status;
}

static int leopard_prepare_message(struct spi_controller *ctlr,
                struct spi_message *msg)
{
    return 0;
}

/**
 * leopard_transfer_one - Initiates the SPI transfer
 * @ctlr:	Pointer to spi_controller structure
 * @spi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provides
 *		information about next transfer parameters
 *
 * This function in master mode fills the TX FIFO, starts the SPI transfer and
 * returns a positive transfer count so that core will wait for completion.
 * This function in slave mode fills the TX FIFO and wait for transfer trigger.
 *
 * Return:	Number of bytes transferred in the last transfer
 */
static int leopard_transfer_one(struct spi_controller *ctlr,
                 struct spi_device *spi,
                 struct spi_transfer *transfer)
{
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);
    u8* rx;
    u8* tx;
    u16 rx_len;
    u16 tx_len;
    u16 tx_to_write;
    u16 rx_to_read;
    int tx_head;
    int tx_tail;
    int rx_head;
    int rx_tail;
    int i;

    rx = transfer->rx_buf;
    tx = transfer->tx_buf;

    rx_len = rx[0] | (rx[1] << 8);
    tx_len = tx[0] | (tx[1] << 8);

    if (tx_len > 0) {
        tx_head = xspi->txbuf.head;
        tx_tail = READ_ONCE(xspi->txbuf.tail);

        tx_to_write = min(tx_len, CIRC_SPACE(tx_head, tx_tail, BUFFER_SIZE));

        for (i = 0; i < tx_to_write; i++) {
            xspi->txbuf.buf[tx_head] = tx[2 + i];
            tx_head = (tx_head + 1) & (BUFFER_SIZE - 1);
        }
        smp_store_release(&xspi->txbuf.head, tx_head);

        tx[0] = tx_to_write & 0xff;
        tx[1] = (tx_to_write >> 8) & 0xff;

        leopard_spi_write(xspi, LEOPARD_SPI_IER, LEOPARD_SPI_IXR_TXFULL);
    }

    if (rx_len > 0) {
        rx_head = smp_load_acquire(&xspi->rxbuf.head);
        rx_tail = xspi->rxbuf.tail;

        rx_to_read = min(rx_len, CIRC_CNT(rx_head, rx_tail, BUFFER_SIZE));

        for (i = 0; i < rx_to_read; i++) {
            rx[2 + i] = xspi->rxbuf.buf[rx_tail];
            rx_tail = (rx_tail + 1) & (BUFFER_SIZE - 1);
        }

        smp_store_release(&xspi->rxbuf.tail, rx_tail);

        rx[0] = rx_to_read & 0xff;
        rx[1] = (rx_to_read >> 8) & 0xff;
    }

    spi_finalize_current_transfer(ctlr);

    return transfer->len;
}

/**
 * leopard_prepare_transfer_hardware - Prepares hardware for transfer.
 * @ctlr:	Pointer to the spi_controller structure which provides
 *		information about the controller.
 *
 * This function enables SPI controller.
 *
 * Return:	0 always
 */
static int leopard_prepare_transfer_hardware(struct spi_controller *ctlr)
{
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);

    leopard_spi_write(xspi, LEOPARD_SPI_ER, LEOPARD_SPI_ER_ENABLE);

    return 0;
}

/**
 * leopard_unprepare_transfer_hardware - Relaxes hardware after transfer
 * @ctlr:	Pointer to the spi_controller structure which provides
 *		information about the controller.
 *
 * Return:	0 always
 */
static int leopard_unprepare_transfer_hardware(struct spi_controller *ctlr)
{
    return 0;
}

/**
 * leopard_spi_detect_fifo_depth - Detect the FIFO depth of the hardware
 * @xspi:	Pointer to the leopard_spi structure
 *
 * The depth of the TX FIFO is a synthesis configuration parameter of the SPI
 * IP. The FIFO threshold register is sized so that its maximum value can be the
 * FIFO size - 1. This is used to detect the size of the FIFO.
 */
static void leopard_spi_detect_fifo_depth(struct leopard_spi *xspi)
{
    /* The MSBs will get truncated giving us the size of the FIFO */
    leopard_spi_write(xspi, LEOPARD_SPI_THLD, 0xffff);
    xspi->tx_fifo_depth = leopard_spi_read(xspi, LEOPARD_SPI_THLD) + 1;

    /* Reset to default */
    leopard_spi_write(xspi, LEOPARD_SPI_THLD, 0x1);
}

/**
 * leopard_spi_probe - Probe method for the SPI driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function initializes the driver data structures and the hardware.
 *
 * Return:	0 on success and error value on error
 */
static int leopard_spi_probe(struct platform_device *pdev)
{
    int ret = 0, irq;
    struct spi_controller *ctlr;
    struct leopard_spi *xspi;
    u32 num_cs;

    ctlr = spi_alloc_slave(&pdev->dev, sizeof(*xspi));
    dev_info(&pdev->dev, "Leopard SPI Probe");

    if (!ctlr)
        return -ENOMEM;

    xspi = spi_controller_get_devdata(ctlr);
    ctlr->dev.of_node = pdev->dev.of_node;
    platform_set_drvdata(pdev, ctlr);

    xspi->regs = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(xspi->regs)) {
        ret = PTR_ERR(xspi->regs);
        goto remove_ctlr;
    }

    xspi->pclk = devm_clk_get(&pdev->dev, "pclk");
    if (IS_ERR(xspi->pclk)) {
        dev_err(&pdev->dev, "pclk clock not found.\n");
        ret = PTR_ERR(xspi->pclk);
        goto remove_ctlr;
    }

    ret = clk_prepare_enable(xspi->pclk);
    if (ret) {
        dev_err(&pdev->dev, "Unable to enable APB clock.\n");
        goto remove_ctlr;
    }

    xspi->ref_clk = devm_clk_get(&pdev->dev, "ref_clk");
    if (IS_ERR(xspi->ref_clk)) {
        dev_err(&pdev->dev, "ref_clk clock not found.\n");
        ret = PTR_ERR(xspi->ref_clk);
        goto clk_dis_apb;
    }

    ret = clk_prepare_enable(xspi->ref_clk);
    if (ret) {
        dev_err(&pdev->dev, "Unable to enable device clock.\n");
        goto clk_dis_apb;
    }

    xspi->irq = devm_gpiod_get(&pdev->dev, "irq", GPIOD_OUT_LOW);
    dev_dbg(&pdev->dev, "GPIO IRQ %d", xspi->irq);
    if (IS_ERR(xspi->irq)) {
        dev_err(&pdev->dev, "irq gpio not found.\n");
        ret = PTR_ERR(xspi->ref_clk);
        goto clk_dis_apb;
    }
    ret = gpiod_direction_output(xspi->irq, 0);
    if (ret < 0) {
        dev_err(&pdev->dev, "irq gpio configuration error.\n");
        goto clk_dis_apb;
    }

    leopard_spi_detect_fifo_depth(xspi);

    /* SPI controller initializations */
    leopard_spi_init_hw(xspi);

    irq = platform_get_irq(pdev, 0);
    if (irq <= 0) {
        ret = -ENXIO;
        goto clk_dis_all;
    }

    ret = devm_request_irq(&pdev->dev, irq, leopard_spi_irq,
                   0, pdev->name, ctlr);
    if (ret != 0) {
        ret = -ENXIO;
        dev_err(&pdev->dev, "request_irq failed\n");
        goto clk_dis_all;
    }

    ctlr->use_gpio_descriptors = true;
    ctlr->prepare_transfer_hardware = leopard_prepare_transfer_hardware;
    ctlr->prepare_message = leopard_prepare_message;
    ctlr->transfer_one = leopard_transfer_one;
    ctlr->unprepare_transfer_hardware = leopard_unprepare_transfer_hardware;
    ctlr->mode_bits = SPI_CPOL | SPI_CPHA;
    ctlr->bits_per_word_mask = SPI_BPW_MASK(8);

    ctlr->mode_bits |= SPI_NO_CS;
    xspi->txbuf.buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    xspi->rxbuf.buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    xspi->rxbuf.head = xspi->rxbuf.tail = 0;
    xspi->txbuf.head = xspi->txbuf.tail = 0;
    xspi->tx_bytes = 0;
    xspi->rx_bytes = 0;
    xspi->zeroes = 0;
    leopard_spi_write(xspi, LEOPARD_SPI_THLD, xspi->tx_fifo_depth - 1);
    leopard_spi_write(xspi, LEOPARD_SPI_IER, LEOPARD_SPI_IXR_RXNEMTY | LEOPARD_SPI_IXR_TXOW);

    ret = spi_register_controller(ctlr);
    if (ret) {
        dev_err(&pdev->dev, "spi_register_controller failed\n");
        goto clk_dis_all;
    }

    return ret;

clk_dis_all:
clk_dis_apb:
    clk_disable_unprepare(xspi->pclk);
remove_ctlr:
    spi_controller_put(ctlr);
    return ret;
}

/**
 * leopard_spi_remove - Remove method for the SPI driver
 * @pdev:	Pointer to the platform_device structure
 *
 * This function is called if a device is physically removed from the system or
 * if the driver module is being unloaded. It frees all resources allocated to
 * the device.
 *
 * Return:	0 on success and error value on error
 */
static int leopard_spi_remove(struct platform_device *pdev)
{
    struct spi_controller *ctlr = platform_get_drvdata(pdev);
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);

    leopard_spi_write(xspi, LEOPARD_SPI_ER, LEOPARD_SPI_ER_DISABLE);

    clk_disable_unprepare(xspi->ref_clk);
    clk_disable_unprepare(xspi->pclk);
    pm_runtime_set_suspended(&pdev->dev);
    pm_runtime_disable(&pdev->dev);

    spi_unregister_controller(ctlr);

    kfree(xspi->txbuf.buf);
    kfree(xspi->rxbuf.buf);

    return 0;
}

/**
 * leopard_spi_suspend - Suspend method for the SPI driver
 * @dev:	Address of the platform_device structure
 *
 * This function disables the SPI controller and
 * changes the driver state to "suspend"
 *
 * Return:	0 on success and error value on error
 */
static int __maybe_unused leopard_spi_suspend(struct device *dev)
{
    struct spi_controller *ctlr = dev_get_drvdata(dev);

    return spi_controller_suspend(ctlr);
}

/**
 * leopard_spi_resume - Resume method for the SPI driver
 * @dev:	Address of the platform_device structure
 *
 * This function changes the driver state to "ready"
 *
 * Return:	0 on success and error value on error
 */
static int __maybe_unused leopard_spi_resume(struct device *dev)
{
    struct spi_controller *ctlr = dev_get_drvdata(dev);
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);

    leopard_spi_init_hw(xspi);
    return spi_controller_resume(ctlr);
}

/**
 * leopard_spi_runtime_resume - Runtime resume method for the SPI driver
 * @dev:	Address of the platform_device structure
 *
 * This function enables the clocks
 *
 * Return:	0 on success and error value on error
 */
static int __maybe_unused leopard_spi_runtime_resume(struct device *dev)
{
    struct spi_controller *ctlr = dev_get_drvdata(dev);
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);
    int ret;

    ret = clk_prepare_enable(xspi->pclk);
    if (ret) {
        dev_err(dev, "Cannot enable APB clock.\n");
        return ret;
    }

    ret = clk_prepare_enable(xspi->ref_clk);
    if (ret) {
        dev_err(dev, "Cannot enable device clock.\n");
        clk_disable_unprepare(xspi->pclk);
        return ret;
    }
    return 0;
}

/**
 * leopard_spi_runtime_suspend - Runtime suspend method for the SPI driver
 * @dev:	Address of the platform_device structure
 *
 * This function disables the clocks
 *
 * Return:	Always 0
 */
static int __maybe_unused leopard_spi_runtime_suspend(struct device *dev)
{
    struct spi_controller *ctlr = dev_get_drvdata(dev);
    struct leopard_spi *xspi = spi_controller_get_devdata(ctlr);

    clk_disable_unprepare(xspi->ref_clk);
    clk_disable_unprepare(xspi->pclk);

    return 0;
}

static const struct dev_pm_ops leopard_spi_dev_pm_ops = {
    SET_RUNTIME_PM_OPS(leopard_spi_runtime_suspend,
               leopard_spi_runtime_resume, NULL)
    SET_SYSTEM_SLEEP_PM_OPS(leopard_spi_suspend, leopard_spi_resume)
};

static const struct of_device_id leopard_spi_of_match[] = {
    { .compatible = "kp,leopard-spi" },
    { /* end of table */ }
};
MODULE_DEVICE_TABLE(of, leopard_spi_of_match);

/* leopard_spi_driver - This structure defines the SPI subsystem platform driver */
static struct platform_driver leopard_spi_driver = {
    .probe	= leopard_spi_probe,
    .remove	= leopard_spi_remove,
    .driver = {
        .name = LEOPARD_SPI_NAME,
        .of_match_table = leopard_spi_of_match,
        .pm = &leopard_spi_dev_pm_ops,
    },
};

module_platform_driver(leopard_spi_driver);

MODULE_AUTHOR("KP Labs");
MODULE_DESCRIPTION("Leopard SPI slave driver");
MODULE_LICENSE("GPL");
