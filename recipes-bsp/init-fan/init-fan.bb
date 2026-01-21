SUMMARY = "Fan - enable automatic speed control"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "^(leopard-ebb)$"

# TODO Don't hardcode hwmon instance https://kplabs.atlassian.net/browse/LPS-24

inherit systemd

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "init-fan.service"

SRC_URI = " \
    file://init-fan.service \
    file://init-fan \
"

do_install() {
    install -d ${D}/${sbindir}
    install -m 0755 ${WORKDIR}/init-fan ${D}/${sbindir}

    install -d ${D}/${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/init-fan.service ${D}/${systemd_system_unitdir}
}

FILES:${PN} = " \
    ${sbindir}/init-fan \
    ${systemd_system_unitdir}/init-fan.service \
"
