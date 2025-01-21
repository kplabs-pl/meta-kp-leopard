FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

KMACHINE:leopard-ebb ?= 'leopard-ebb'
KMACHINE:leopard-dpu ?= 'leopard-dpu'

SRC_URI:append = " \
    file://disable_unused_kernel_features.cfg \
    file://sata.cfg \
"

SRC_URI:append:leopard-ebb = " \
    file://fan-controller-support.cfg \
"

SRC_URI:append:leopard-dpu = " \
    file://0001-leopard-spi.patch \
    file://spi-slave.cfg \
    file://spidev-leopard.h \
    file://spidev-leopard.c \
    file://spi-leopard.c \
    file://gpio-aggregator.cfg \
    file://0001-add-kplabs-gpio-aggregator.patch \
"

do_patch:append:leopard-dpu() {
    cp ${WORKDIR}/spidev-leopard.h ${S}/include/uapi/linux/spi/spidev-leopard.h
    cp ${WORKDIR}/spidev-leopard.c ${S}/drivers/spi/spidev-leopard.c
    cp ${WORKDIR}/spi-leopard.c ${S}/drivers/spi/spi-leopard.c
}

KERNEL_IMAGETYPES += "Image.lzma"
