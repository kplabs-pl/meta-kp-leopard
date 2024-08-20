FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI = " \
    file://si5338.dtsi \
    file://sata.dtsi \
    file://nand-controller.dtsi \
    file://qspi-partitions.dtsi \
"

SRC_URI:append:leopard-dpu = " \
    file://dpu-qspi-controller.dtsi \
    file://dpu-sata-control.dtsi \
    file://dpu-ethernet.dtsi \
"

SRC_URI:append:leopard-ebb = " \
    file://fan-controller.dtsi \
    file://ebb-qspi-controller.dtsi \
    file://ebb-sata-control.dtsi \
    file://ebb-ethernet.dtsi \
"

EXTRA_OVERLAYS:append = " \
    si5338.dtsi \
    sata.dtsi \
    nand-controller.dtsi \
    qspi-partitions.dtsi \
"

EXTRA_OVERLAYS:append:leopard-dpu = " \
    dpu-qspi-controller.dtsi \
    dpu-sata-control.dtsi \
    dpu-ethernet.dtsi \
"

EXTRA_OVERLAYS:append:leopard-ebb = " \
    fan-controller.dtsi \
    ebb-qspi-controller.dtsi \
    ebb-sata-control.dtsi \
    ebb-ethernet.dtsi \
"
