LICENSE = "CLOSED"

inherit base-leopard-image

IMAGE_INSTALL:append:leopard-dpu = " \
    leopard-communication-hub \
"

IMAGE_INSTALL:append:leopard-ebb = " \
    init-fan \
    libusb1 \
"