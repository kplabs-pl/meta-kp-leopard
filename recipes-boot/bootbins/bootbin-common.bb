inherit bootbin

BIN_KIND = "common"

DEPENDS += " \
    virtual/fsbl \
    virtual/pmu-firmware \
    virtual/bootloader \
    device-tree \
    virtual/arm-trusted-firmware \
"

PROVIDES = "virtual/boot-bin"

BIF_PARTITION_ATTR = "fsbl pmufw trusted-firmware-a device-tree u-boot-xlnx"

BIF_PARTITION_ATTR[fsbl]="bootloader, destination_cpu=a53-0"
BIF_PARTITION_IMAGE[fsbl]="${DEPLOY_DIR_IMAGE}/fsbl-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[fsbl]="virtual/fsbl:do_deploy"

BIF_PARTITION_ATTR[pmufw]="destination_cpu=pmu"
BIF_PARTITION_IMAGE[pmufw]="${DEPLOY_DIR_IMAGE}/pmu-firmware-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[pmufw]="virtual/pmu-firmware:do_deploy"

BIF_PARTITION_ATTR[trusted-firmware-a]="destination_cpu=a53-0,exception_level=el-3,trustzone"
BIF_PARTITION_IMAGE[trusted-firmware-a]="${DEPLOY_DIR_IMAGE}/arm-trusted-firmware.elf"
BIF_PARTITION_DEPENDS[trusted-firmware-a]="virtual/arm-trusted-firmware:do_deploy"

BIF_PARTITION_ATTR[device-tree] ?= "destination_cpu=a53-0,load=0x100000"
BIF_PARTITION_IMAGE[device-tree] ?= "${DEPLOY_DIR_IMAGE}/devicetree/system-top.dtb"
BIF_PARTITION_DEPENDS[device-tree]="virtual/device-tree:do_deploy"

BIF_PARTITION_ATTR[u-boot-xlnx]="destination_cpu=a53-0,exception_level=el-2"
BIF_PARTITION_IMAGE[u-boot-xlnx]="${DEPLOY_DIR_IMAGE}/u-boot-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[u-boot-xlnx]="virtual/bootloader:do_deploy"
