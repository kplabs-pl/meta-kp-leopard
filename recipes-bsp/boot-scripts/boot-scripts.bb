LICENSE = "CLOSED"

SRC_URI = " \
    file://sd-boot.cmd \
    file://tftp-boot.cmd \
    file://nand-image0-boot.cmd \
    file://tftp-boot-safe.cmd \
    file://nand-boot.cmd \
"

SRC_URI:append:leopard-ebb = " \
    file://ebb-leopard-boot-flash.cmd \
"

SRC_URI:append:leopard-dpu = " \
    file://dpu-leopard-boot-flash.cmd \
"


S = "${WORKDIR}"

inherit u-boot-script

PROJECT_NAME[doc] = "Full name of the project. It is used to create the TFTP_ROOT variable."

python __anonymous() {
    if d.getVar("PROJECT_NAME", True) in ["", None]:
        bb.fatal("PROJECT_NAME is not set, provide it in project configuration file (kas file)")
}

TFTP_ROOT ?= "${@'/${PROJECT_NAME}/' + '/'.join('${DEPLOY_DIR_IMAGE}'.split('/')[-5:])}"
TFTP_ROOT[doc] = "Path to the root directory of the TFTP server. Default path is resolved from the DEPLOY_DIR_IMAGE variable."

do_configure:prepend(){
    for script in ${S}/*.cmd; do
        sed -i -e 's:@@TFTP_ROOT@@:${TFTP_ROOT}:g' $script
        sed -i -e 's:@@MACHINE@@:${MACHINE}:g' $script
    done
}

do_install:append() {
    install -d ${D}/boot
    install -m 0644 ${B}/nand-boot.scr ${D}/boot/boot.scr
}

FILES:${PN} = "/boot/boot.scr"