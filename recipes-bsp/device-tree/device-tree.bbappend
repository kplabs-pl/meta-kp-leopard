FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI = " \
    file://si5338.dtsi \
    file://sata.dtsi \
    file://nand-controller.dtsi \
    file://boot-flash-partitions.dtsi \
"

SRC_URI:append:leopard-dpu = " \
    file://dpu-boot-flash-controller.dtsi \
    file://dpu-sata-control.dtsi \
    file://dpu-ethernet.dtsi \
    file://leopard-pn-id.dtsi \
    file://remove-spi0.dtsi \
"

SRC_URI:append:leopard-ebb = " \
    file://fan-controller.dtsi \
    file://ebb-boot-flash-controller.dtsi \
    file://ebb-sata-control.dtsi \
"

EXTRA_DT_INCLUDE_FILES:append = " \
    si5338.dtsi \
    sata.dtsi \
    nand-controller.dtsi \
    boot-flash-partitions.dtsi \
"

EXTRA_DT_INCLUDE_FILES:append:leopard-dpu = " \
    dpu-boot-flash-controller.dtsi \
    dpu-sata-control.dtsi \
    dpu-ethernet.dtsi \
    leopard-pn-id.dtsi \
    remove-spi0.dtsi \
"

EXTRA_DT_INCLUDE_FILES:append:leopard-ebb = " \
    fan-controller.dtsi \
    ebb-boot-flash-controller.dtsi \
    ebb-sata-control.dtsi \
"

EXTRA_DT_FILES:append:leopard-dpu = " \
    overlay/leopard-spi-slave-overlay.dtsi \
"

YAML_ENABLE_DT_OVERLAY = "1"
