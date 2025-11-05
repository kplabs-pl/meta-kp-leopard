LICENSE = "CLOSED"

SRC_URI = " \
    file://leopard-pn-id.rules \
    file://detect-pn.cpp \
    file://leopard-detect-pn.service \
"

DEPENDS = " \
    libgpiod \
"

S = "${WORKDIR}"

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "leopard-detect-pn.service"

inherit udev-rules systemd

do_compile() {
    ${CXX} ${CXXFLAGS} -std=c++17 ${WORKDIR}/detect-pn.cpp -c -o ${B}/detect-pn.o
    ${CXX} ${LDFLAGS} ${B}/detect-pn.o -lgpiodcxx -o ${B}/detect-pn
}

do_install:append() {
    install -d ${D}${libexecdir}
    install -m 0755 ${B}/detect-pn ${D}${libexecdir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/leopard-detect-pn.service ${D}${systemd_system_unitdir}
}
