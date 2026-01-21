setenv R @@TFTP_ROOT@@

echo "Running Leopard tftp-boot-safe.scr"
echo "TFTP Root: ${R}"

dhcp

tftpboot ${image_addr_r} ${R}/safe-image-fit-1.0-r0-@@MACHINE@@.fitimage

setenv bootargs "console=ttyPS0,115200 earlycon ro rdinit=/sbin/init"
bootm ${image_addr_r}