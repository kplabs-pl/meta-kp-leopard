SUMMARY = "Leopard SPI Slave service"
LICENSE = "CLOSED"
DESCRIPTION = "Leopard SPI Slave service which loads devicetree overlay after kernel \
modules with leopard-spidev and leopard-spi are loaded \
"

COMPATIBLE_MACHINE = "^(leopard-dpu)$"

FILESEXTRAPATHS:prepend := "${THISDIR}/rules.d:"

inherit systemd udev-rules

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = " \
    leopard-spi-slave.service \
"

SRC_URI = " \
    file://leopard-spi-slave.service \
    file://01-spi-slave.rules \
"

FILES:${PN} += " \
    ${systemd_system_unitdir}/leopard-spi-slave.service \
    ${sysconfdir}/udev/rules.d/01-spi-slave.rules \
"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}

    install -m 0644 ${WORKDIR}/leopard-spi-slave.service ${D}${systemd_system_unitdir}
}

RDEPENDS:${PN} = " \
    spi-leopard-mod \
    spidev-leopard-mod \
"

RPROVIDES:${PN} = " \
    leopard-spi-slave \
"

IMAGE_INSTALL:append = " \
    kernel-module-spi-leopard \
    kernel-module-spidev-leopard \
"