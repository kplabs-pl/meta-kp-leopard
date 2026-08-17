SUMMARY = "Static eth interface name"
LICENSE = "CLOSED"

SRC_URI = " \
    file://10-static-eth-interface-name.link \
"

do_install() {
    install -d ${D}${systemd_unitdir}/network
    install -m 0644 ${WORKDIR}/10-static-eth-interface-name.link ${D}${systemd_unitdir}/network
}

FILES:${PN} = " \
    ${systemd_unitdir}/network/10-static-eth-interface-name.link \
"
