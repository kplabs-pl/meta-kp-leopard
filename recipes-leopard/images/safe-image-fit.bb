LICENSE = "CLOSED"

inherit fitimage

DEPENDS += " \
    virtual/kernel \
    device-tree \
    safe-image \
"

FITIMAGE_SLOTS = "kernel fdt ramdisk"

FITIMAGE_SLOT_kernel = "virtual/kernel"
FITIMAGE_SLOT_kernel[type] = "kernel"
FITIMAGE_SLOT_kernel[file] = "Image.lzma"
FITIMAGE_SLOT_kernel[comp] = "lzma"
FITIMAGE_SLOT_kernel[depends] = "virtual/kernel:do_deploy"

FITIMAGE_SLOT_fdt = "device-tree"
FITIMAGE_SLOT_fdt[type] = "fdt"
FITIMAGE_SLOT_fdt[file] = "${MACHINE}-system.dtb"
FITIMAGE_SLOT_fdt[depends] = "device-tree:do_deploy"

FITIMAGE_SLOT_ramdisk = "safe-image"
FITIMAGE_SLOT_ramdisk[type] = "ramdisk"
FITIMAGE_SLOT_ramdisk[fstype] = "rootfs.cpio.lzma"
FITIMAGE_SLOT_ramdisk[depends] = "safe-image:do_image_complete"

FITIMAGE_ENTRYPOINT = "0x80000"
FITIMAGE_LOADADDRESS = "0x80000"
