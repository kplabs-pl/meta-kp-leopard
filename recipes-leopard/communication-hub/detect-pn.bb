LICENSE = "CLOSED"

SRC_URI = " \
    file://leopard-pn-id.rules \
    file://detect-pn.cpp \
    file://leopard-detect-pn.service \
    file://get_network_address_pn1.py \
"

DEPENDS = " \
    libgpiod \
    leopard-option-sheet \
"

S = "${WORKDIR}"

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "leopard-detect-pn.service"

inherit udev-rules systemd python3native

do_compile() {
    LEOPARD_NETWORK_ADDRESS_PN1=$(nativepython3 ${WORKDIR}/get_network_address_pn1.py ${RECIPE_SYSROOT}/etc/leopard/option-sheet.toml)

    ${CXX} ${CXXFLAGS} \
        -std=c++17 \
        -DLEOPARD_NETWORK_ADDRESS_PN1=$LEOPARD_NETWORK_ADDRESS_PN1 \
        ${WORKDIR}/detect-pn.cpp \
        -c -o ${B}/detect-pn.o

    ${CXX} ${LDFLAGS} ${B}/detect-pn.o -lgpiodcxx -o ${B}/detect-pn
}

do_install:append() {
    install -d ${D}${libexecdir}
    install -m 0755 ${B}/detect-pn ${D}${libexecdir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/leopard-detect-pn.service ${D}${systemd_system_unitdir}
}
