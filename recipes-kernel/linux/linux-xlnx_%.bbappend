FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

KMACHINE:leopard-ebb ?= 'leopard-ebb'
KMACHINE:leopard-dpu ?= 'leopard-dpu'

SRC_URI:append = " \
    file://disable_unused_kernel_features.cfg \
    file://sata.cfg \
    file://spi-slave.cfg \
"

SRC_URI:append:leopard-ebb = " \
    file://fan-controller-support.cfg \
"

SRC_URI:append:leopard-dpu = " \
    file://gpio-aggregator.cfg \
    file://0001-add-kplabs-gpio-aggregator.patch \
"

KERNEL_IMAGETYPES += "Image.lzma"
