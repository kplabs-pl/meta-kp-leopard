SUMMARY = "Leopard Base image"
LICENSE = "CLOSED"

inherit core-image image-identification

DEPENDS += " \
    device-tree \
    os-release \
"

TOOLCHAIN_HOST_TASK += " nativesdk-cmake"

TOOLCHAIN_TARGET_TASK += " \
    nng-dev \
    libcsp-dev \
"

EXTRA_IMAGE_FEATURES = "debug-tweaks"
IMAGE_INSTALL:append = " \
    leopard-udev-rules \
    leopard-scripts \
    boot-scripts \
    openssh \
    libstdc++ \
    mtd-utils \
    mtd-utils-ubifs \
    libgcc \
    libatomic \
    screen \
    ethtool \
    i2c-tools \
    stress-ng \
    smartmontools \
    fpga-manager-script \
    e2fsprogs-mke2fs \
    hdparm \
    parted \
    static-eth-interface-name \
"

IMAGE_FSTYPES = "cpio.gz.u-boot ubi tar.gz"

UBI_VOLNAME = "rootfs"
MKUBIFS_ARGS += "-m 8192 -e 1008KiB -c 900"
UBINIZE_ARGS += "-m 8192 -p 1MiB"
