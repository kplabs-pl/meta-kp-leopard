SUMMARY = "Leopard EBB image"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "^(leopard-ebb)$"
inherit base-leopard-image

IMAGE_INSTALL:append = " \
    init-fan \
    libusb1 \
"

