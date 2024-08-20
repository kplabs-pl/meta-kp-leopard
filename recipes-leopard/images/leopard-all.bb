LICENSE = "CLOSED"

inherit nopackages

DEPENDS:append = "\
    virtual/fsbl \
    virtual/pmu-firmware \
    virtual/bootloader \
    arm-trusted-firmware \
    device-tree \
    virtual/kernel \
    boot-scripts \
    safe-image \
    safe-image-fit \
    bootbin-common \
"

DEPENDS:append:leopard-dpu = "\
    dpu-leopard \
"

DEPENDS:append:leopard-ebb = "\
    ebb-leopard \
"