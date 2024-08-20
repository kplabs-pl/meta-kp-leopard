
echo "Running Leopard sd-boot.scr"

setenv bootargs "console=ttyPS0,115200 earlycon root=/dev/mmcblk0p2 ro rootwait"

if test -e ${devtype} ${devnum}:${distro_bootpart} /Image; then
    fatload ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} Image;
fi

if test -e ${devtype} ${devnum}:${distro_bootpart} /system.dtb; then
    fatload ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} system.dtb;
fi

if test -e ${devtype} ${devnum}:${distro_bootpart} /@@BOARD_NAME@@-leopard-leopard-@@BOARD_NAME@@.rootfs.cpio.gz.u-boot; then
    fatload ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} @@BOARD_NAME@@-leopard-leopard-@@BOARD_NAME@@.rootfs.cpio.gz.u-boot;
    booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}
    exit;
fi

booti ${kernel_addr_r} - ${fdt_addr_r}
exit;