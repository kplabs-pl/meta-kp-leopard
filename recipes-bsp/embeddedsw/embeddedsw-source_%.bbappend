FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://0001-enable_clock_generator.patch \
    file://0002-Change-QSPI-clock-prescaler.patch \
"


FILESEXTRAPATHS:prepend := "${THISDIR}/clock-generator:"
SRC_URI:append = " \
    file://kp_clockGenerator_registerConfig.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.c;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.c;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.c;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
"
