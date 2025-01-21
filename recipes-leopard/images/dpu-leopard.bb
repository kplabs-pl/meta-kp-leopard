SUMMARY = "Leopard DPU image"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "^(leopard-dpu)$"
inherit base-leopard-image

IMAGE_INSTALL:append = " \
    leopard-communication-hub \
"