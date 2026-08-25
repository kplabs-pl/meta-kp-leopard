LICENSE = "CLOSED"

SRC_URI = "file://compatible-option-sheet.toml"

LEOPARD_OPTION_SHEET ?= "${WORKDIR}/compatible-option-sheet.toml"

inherit deploy

do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 ${LEOPARD_OPTION_SHEET} ${DEPLOYDIR}/leopard-option-sheet.toml
}

do_install() {
    install -d ${D}/etc/leopard
    install -m 0755 ${LEOPARD_OPTION_SHEET} ${D}/etc/leopard/option-sheet.toml
}

addtask do_deploy before do_build after do_compile

SYSROOT_DIRS += "${sysconfdir}"

do_deploy[file-checksums] += "${LEOPARD_OPTION_SHEET}:True"
do_install[file-checksums] += "${LEOPARD_OPTION_SHEET}:True"