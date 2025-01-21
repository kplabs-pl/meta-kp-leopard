inherit bootbin

BIN_KIND = "common"

DEPENDS += " \
    virtual/fsbl \
    virtual/pmu-firmware \
    virtual/bootloader \
    device-tree \
    arm-trusted-firmware \
"

PROVIDES = "virtual/boot-bin"

BIF_PARTITION_ATTR = "fsbl pmu atf device-tree u-boot"

BIF_PARTITION_ATTR[fsbl]="bootloader, destination_cpu=a53-0"
BIF_PARTITION_IMAGE[fsbl]="${DEPLOY_DIR_IMAGE}/fsbl-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[fsbl]="virtual/fsbl:do_deploy"

BIF_PARTITION_ATTR[pmu]="destination_cpu=pmu"
BIF_PARTITION_IMAGE[pmu]="${DEPLOY_DIR_IMAGE}/pmu-firmware-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[pmu]="virtual/pmu-firmware:do_deploy"

BIF_PARTITION_ATTR[atf]="destination_cpu=a53-0,exception_level=el-3,trustzone"
BIF_PARTITION_IMAGE[atf]="${DEPLOY_DIR_IMAGE}/arm-trusted-firmware.elf"
BIF_PARTITION_DEPENDS[atf]="arm-trusted-firmware:do_deploy"

BIF_PARTITION_ATTR[device-tree] ?= "destination_cpu=a53-0,load=0x100000"
BIF_PARTITION_IMAGE[device-tree] ?= "${DEPLOY_DIR_IMAGE}/devicetree/system-top.dtb"
BIF_PARTITION_DEPENDS[device-tree]="virtual/device-tree:do_deploy"

BIF_PARTITION_ATTR[u-boot]="destination_cpu=a53-0,exception_level=el-2"
BIF_PARTITION_IMAGE[u-boot]="${DEPLOY_DIR_IMAGE}/u-boot-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[u-boot]="virtual/bootloader:do_deploy"
