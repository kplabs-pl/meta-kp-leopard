LICENSE = "CLOSED"
SUMMARY = "udev rules to run on specific events"

inherit udev-rules

FILESEXTRAPATHS:prepend := "${THISDIR}/rules.d:"

SRC_URI = " \
    file://10-ssd.rules \
    file://50-mtd-names.rules \
    file://50-ubi.rules \
    file://60-mount-nand-data-partition.rules \
    file://leopard-ubi-mtd-name \
"

do_install:append() {
    install -d ${D}/${sysconfdir}/udev/rules.d
    install -m 0755 ${WORKDIR}/leopard-ubi-mtd-name ${D}/${sysconfdir}/udev/rules.d/leopard-ubi-mtd-name
}

FILES:${PN}:append = "${sysconfdir}/udev/rules.d/*"