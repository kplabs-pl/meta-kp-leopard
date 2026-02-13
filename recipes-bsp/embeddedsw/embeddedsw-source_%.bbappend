FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://0001-enable_clock_generator.patch \
    file://0002-Change-QSPI-clock-prescaler.patch \
"

FILESEXTRAPATHS:prepend := "${THISDIR}/clock-generator:"

KP_CLOCK_GENERATOR_CONFIG_FILE ?= "kp_clockGenerator_registerConfig.h"
KP_CLOCK_GENERATOR_CONFIG_FILE[doc] = "File with SI5338 clock generator configuration, generated with ClockBuilder Pro"

SRC_URI:append = " \
    file://${KP_CLOCK_GENERATOR_CONFIG_FILE};subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.c.template;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.c;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.c;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.h;subdir=${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/ \
"

do_include_generator_config_file_in_sources(){
    sed -i -e 's:@@CLOCK_CONFIG_FILE@@:${KP_CLOCK_GENERATOR_CONFIG_FILE}:' ${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/kp_clockGenerator.c.template
    mv ${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/kp_clockGenerator.c.template ${SHARED_S}/lib/sw_apps/zynqmp_fsbl/src/kp_clockGenerator.c
}

addtask include_generator_config_file_in_sources after do_unpack before do_configure