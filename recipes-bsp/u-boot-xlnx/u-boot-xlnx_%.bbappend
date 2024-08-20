FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://atheros-phy-support.cfg \
    file://disable-autoload.cfg \
    file://fast-tftp.cfg \
    file://change-boot-script-offset.cfg \
    file://sfdp.cfg \
    file://version.cfg \
"

# Setting UBOOT_LOCALVERSION directly causes bitbake error `The metadata is not deterministic and this needs to be fixed.`
# This is a workaround to that issue.
LOCALVERSION := "-build-${@bb.process.run('git rev-parse HEAD')[0].strip()}"
do_configure:append(){
    sed -i -e "s:@@LOCALVERSION@@:${LOCALVERSION}:" ${B}/.config
}