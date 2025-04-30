SUMMARY = "Communication Hub"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "^(leopard-dpu)$"

inherit systemd

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = " \
    communication-hub-spi.service \
    service-filesystem.service \
    service-job-runner.service \
"

# TODO comm hub binaries are locally build in Release mode: https://kplabs.atlassian.net/browse/LPS-406
SRC_URI = " \
    file://communication-hub.env \
    file://bin/communication_hub_spi \
    file://bin/service_filesystem \
    file://bin/service_job_runner \
    file://communication-hub-spi.service \
    file://communication-hub-setup.service \
    file://service-filesystem.service \
    file://service-job-runner.service \
"


do_install() {
    install -d ${D}/etc/default/opt
    install -m 0755 ${WORKDIR}/communication-hub.env ${D}/etc/default/opt

    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/bin/communication_hub_spi ${D}${bindir}
    install -m 0755 ${WORKDIR}/bin/service_filesystem ${D}${bindir}
    install -m 0755 ${WORKDIR}/bin/service_job_runner ${D}${bindir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/communication-hub-spi.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/communication-hub-setup.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/service-filesystem.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/service-job-runner.service ${D}${systemd_system_unitdir}
}

FILES:${PN} += " \
    ${bindir}/communication_hub_spi \
    ${bindir}/service_filesystem \
    ${bindir}/service_job_runner \
    ${systemd_system_unitdir}/communication-hub-spi.service \
    ${systemd_system_unitdir}/communication-hub-setup.service \
    ${systemd_system_unitdir}/service-filesystem.service \
    ${systemd_system_unitdir}/service-job-runner.service \
"

RDEPENDS:${PN} = " \
    libnsl2 \
    detect-pn \
    leopard-spi-slave \
    nng \
"

IMAGE_INSTALL:append = " \
    libnsl2 \
"