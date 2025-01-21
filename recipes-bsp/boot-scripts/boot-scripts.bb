LICENSE = "CLOSED"

SRC_URI = " \
    file://sd-boot.cmd \
    file://tftp-boot.cmd \
    file://nand-linux0-boot.cmd \
    file://nand-boot.cmd \
"

SRC_URI:append:leopard-ebb = " \
    file://ebb-leopard-qspi-boot.cmd \
"

SRC_URI:append:leopard-dpu = " \
    file://dpu-leopard-qspi-boot.cmd \
"

BOARD_NAME:leopard-ebb = "ebb"
BOARD_NAME:leopard-dpu = "dpu"


S = "${WORKDIR}"

inherit u-boot-script

python __anonymous() {
    if d.getVar("PROJECT_NAME", True) in ["", None]:
        bb.fatal("PROJECT_NAME is not set, provide it in project configuration file (kas file)")
}

TFTP_ROOT ?= "${@'/${PROJECT_NAME}/' + '/'.join('${DEPLOY_DIR_IMAGE}'.split('/')[-5:])}"

do_configure:prepend(){
    for script in ${S}/*.cmd; do
        sed -i -e 's:@@TFTP_ROOT@@:${TFTP_ROOT}:g' $script
        sed -i -e 's:@@BOARD_NAME@@:${BOARD_NAME}:g' $script
    done
}

do_install:append() {
    install -d ${D}/boot
    install -m 0644 ${B}/nand-boot.scr ${D}/boot/boot.scr
}

FILES:${PN} = "/boot/boot.scr"