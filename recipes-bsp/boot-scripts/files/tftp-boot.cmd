
setenv R @@TFTP_ROOT@@

echo "Running Leopard tftp-boot.scr"
echo "TFTP Root: ${R}"

dhcp

tftpboot ${kernel_addr_r} ${R}/Image
tftpboot ${fdt_addr_r} ${R}/system.dtb
tftpboot ${ramdisk_addr_r} ${R}/nominal-image-@@MACHINE@@.rootfs.cpio.gz.u-boot


setenv bootargs "console=ttyPS0,115200 earlycon ro rdinit=/sbin/init"
booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}