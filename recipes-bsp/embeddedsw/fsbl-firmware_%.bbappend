# If debug output is needed change 0 -> 1.
# This option also enables PMU debug output.
XSCTH_BUILD_DEBUG = "0"

# If debug output is needed add compiler flags:
# "-DFSBL_DEBUG_INFO" - enables debug output.
# "-DFSBL_NAND_EXCLUDE_VAL" - (for example) excludes NAND to reduce FSBL size to fit in OCM,
#   because enabling debug commands increses FSBL size.
YAML_COMPILER_FLAGS:append = "-DFSBL_SECURE_EXCLUDE -DFSBL_NAND_EXCLUDE_VAL"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI:append = " \
    file://0001-enable_clock_generator.patch \
    file://0002-Change-QSPI-clock-prescaler.patch \
"

FILESEXTRAPATHS:prepend := "${THISDIR}/clock-generator:"
SRC_URI:append = " \
    file://kp_clockGenerator_registerConfig.h;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.c;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_clockGenerator.h;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.c;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_si5338.h;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.c;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
    file://kp_zynqI2C.h;subdir=${S}/lib/sw_apps/zynqmp_fsbl/src/ \
"
SRC_URI += "file://0001-Add-support-for-SI5338-clock-generator.patch"

