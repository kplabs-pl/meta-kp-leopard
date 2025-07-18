
echo "Running Leopard sd-boot.scr"

setenv bootargs "console=ttyPS0,115200 earlycon root=/dev/mmcblk0p2 ro rootwait"

if test -e ${devtype} ${devnum}:${distro_bootpart} /Image; then
    echo "/Image found"
    fatload ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} Image;
fi

if test -e ${devtype} ${devnum}:${distro_bootpart} /system.dtb; then
    echo "/system.dtb found"
    fatload ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} system.dtb;
fi

if test -e ${devtype} ${devnum}:${distro_bootpart} /nominal-image-@@MACHINE@@.rootfs.cpio.gz.u-boot; then
    echo "/nominal-image-@@MACHINE@@.rootfs.cpio.gz.u-boot found"
    fatload ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} nominal-image-@@MACHINE@@.rootfs.cpio.gz.u-boot;
    booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}
    exit;
fi

booti ${kernel_addr_r} - ${fdt_addr_r}
exit;