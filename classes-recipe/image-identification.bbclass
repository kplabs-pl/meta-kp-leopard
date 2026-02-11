SUMMARY = "Leopard Base image"
LICENSE = "CLOSED"

ROOTFS_POSTPROCESS_COMMAND:append = " rootfs_update_timestamp; leopard_image_identification;"

rootfs_update_timestamp () {
    date "+%m%d%H%M%Y" >${IMAGE_ROOTFS}/etc/timestamp
}

leopard_image_identification() {
    echo "VARIANT=${IMAGE_BASENAME}-${MACHINE}" >> ${IMAGE_ROOTFS}/etc/os-release
}