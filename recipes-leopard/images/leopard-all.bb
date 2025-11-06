LICENSE = "CLOSED"

inherit nopackages

DEPENDS:append = "\
    safe-image \
    safe-image-fit \
    nominal-image \
    virtual/fsbl \
    virtual/pmu-firmware \
    virtual/bootloader \
    virtual/arm-trusted-firmware \
    device-tree \
    virtual/kernel \
    boot-scripts \
    bootbin-common \
"

do_build[depends] = "nominal-image:do_image_complete"
do_build[depends] = "safe-image:do_image_complete"