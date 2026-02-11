#ifndef LEOPARD_LEOPARD_SPIDEV_H
#define LEOPARD_LEOPARD_SPIDEV_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/spi/spi.h>

/* IOCTL commands */

#define LEOPARD_SPI_IOC_MAGIC			'k'

/**
 * struct leopard_spi_ioc_transfer - describes a single LEOPARD_SPI transfer
 * @tx_buf: Holds pointer to userspace buffer with transmit data, or null.
 *	If no data is provided, zeroes are shifted out.
 * @rx_buf: Holds pointer to userspace buffer for receive data, or null.
 * @len: Length of tx and rx buffers, in bytes.
 * @speed_hz: Temporary override of the device's bitrate.
 * @bits_per_word: Temporary override of the device's wordsize.
 * @delay_usecs: If nonzero, how long to delay after the last bit transfer
 *	before optionally deselecting the device before the next transfer.
 * @cs_change: True to deselect device before starting the next transfer.
 * @word_delay_usecs: If nonzero, how long to wait between words within one
 *	transfer. This property needs explicit support in the LEOPARD_SPI controller,
 *	otherwise it is silently ignored.
 *
 * This structure is mapped directly to the kernel spi_transfer structure;
 * the fields have the same meanings, except of course that the pointers
 * are in a different address space (and may be of different sizes in some
 * cases, such as 32-bit i386 userspace over a 64-bit x86_64 kernel).
 * Zero-initialize the structure, including currently unused fields, to
 * accommodate potential future updates.
 *
 * LEOPARD_SPI_IOC_MESSAGE gives userspace the equivalent of kernel spi_sync().
 * Pass it an array of related transfers, they'll execute together.
 * Each transfer may be half duplex (either direction) or full duplex.
 *
 *	struct leopard_spi_ioc_transfer mesg[4];
 *	...
 *	status = ioctl(fd, LEOPARD_SPI_IOC_MESSAGE(4), mesg);
 *
 * So for example one transfer might send a nine bit command (right aligned
 * in a 16-bit word), the next could read a block of 8-bit data before
 * terminating that command by temporarily deselecting the chip; the next
 * could send a different nine bit command (re-selecting the chip), and the
 * last transfer might write some register values.
 */
struct leopard_spi_ioc_transfer {
    __u64		tx_buf;
    __u64		rx_buf;

    __u32		len;
    __u32		speed_hz;

    __u16		delay_usecs;
    __u8		bits_per_word;
    __u8		cs_change;
    __u8		tx_nbits;
    __u8		rx_nbits;
    __u8		word_delay_usecs;
    __u8		pad;

    /* If the contents of 'struct leopard_spi_ioc_transfer' ever change
     * incompatibly, then the ioctl number (currently 0) must change;
     * ioctls with constant size fields get a bit more in the way of
     * error checking than ones (like this) where that field varies.
     *
     * NOTE: struct layout is the same in 64bit and 32bit userspace.
     */
};

/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define LEOPARD_SPI_MSGSIZE(N) \
    ((((N)*(sizeof (struct leopard_spi_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
        ? ((N)*(sizeof (struct leopard_spi_ioc_transfer))) : 0)
#define LEOPARD_SPI_IOC_MESSAGE(N) _IOW(LEOPARD_SPI_IOC_MAGIC, 0, char[LEOPARD_SPI_MSGSIZE(N)])

#endif /* LEOPARD_LEOPARD_SPIDEV_H */
