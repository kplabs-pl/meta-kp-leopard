DESCRIPTION = "ZynqMP DRAM Test"

inherit xlnx-embeddedsw deploy


# COMPATIBLE_HOST = ".*-(elf|.*eabi)"
COMPATIBLE_MACHINE = "none"
COMPATIBLE_MACHINE:zynq = ".*"
COMPATIBLE_MACHINE:zynqmp = ".*"

PACKAGE_ARCH = "${MACHINE_ARCH}"

# This is the default in most BSPs.  A MACHINE.conf can override this!
DRAM_TEST_IMAGE_NAME ??= "zynqmp-dram-test"

inherit image-artifact-names

DRAM_TEST_BASE_NAME ?= "${DRAM_TEST_IMAGE_NAME}-${PKGE}-${PKGV}-${PKGR}-${MACHINE}${IMAGE_VERSION_SUFFIX}"

ESW_COMPONENT ??= "zynqmp_dram_test.elf"

# Disable buildpaths QA check warnings.
INSANE_SKIP:${PN} += "buildpaths"

# FSBL must be successfully started in order to run this application
DEPENDS = " \
    virtual/fsbl \
    virtual/pmu-firmware \
"

include zynqmp-dram-test.inc