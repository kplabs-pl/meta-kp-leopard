FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://atheros-phy-support.cfg \
    file://disable-autoload.cfg \
    file://fast-tftp.cfg \
    file://change-boot-script-offset.cfg \
    file://sfdp.cfg \
    file://version.cfg \
    file://enable-misc-init-r.cfg \
    file://nand_max_chips.cfg \
    file://misc.c;subdir=${S}/arch/arm/mach-zynqmp/ \
    file://0001-Support-setting-MAC-address-based-on-PN-version-runn.patch \
"

# Setting UBOOT_LOCALVERSION directly causes bitbake error `The metadata is not deterministic and this needs to be fixed.`
# This is a workaround to that issue.
LOCALVERSION := "-build-${@bb.process.run('git rev-parse HEAD || echo unknown')[0].strip()}"
do_configure:append(){
    sed -i -e "s:@@LOCALVERSION@@:${LOCALVERSION}:" ${B}/.config
}

EXTRA_OEMAKE:append = " KCFLAGS=' -DCONFIG_TFTP_FILE_NAME_MAX_LEN=256'"