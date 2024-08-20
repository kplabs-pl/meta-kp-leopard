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

KERNEL_IMAGETYPES += "Image.lzma"
