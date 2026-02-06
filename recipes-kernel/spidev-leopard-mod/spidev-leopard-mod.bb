SUMMARY = "SPIDEV Leopard kernel module"
DESCRIPTION = "${SUMMARY}"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
    file://COPYING \
    file://Makefile \
    file://spidev-leopard.c \
    file://spidev-leopard.h \
"

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " spidev-leopard"
