SRC_URI:append = " \
    ${EMBEDDEDSW_SRCURI} \
"

# That's a terrible hack, but xilinx's layers don't care about psu_init.c from delivered *.xsa and generate one themselves.
do_configure:append() {
    cp ${B}/fsbl-firmware_plat/hw/psu_init.c ${B}/fsbl-firmware/psu_init.c
}