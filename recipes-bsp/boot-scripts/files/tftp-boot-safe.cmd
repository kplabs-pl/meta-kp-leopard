setenv R @@TFTP_ROOT@@

echo "Running Leopard tftp-boot-safe.scr"
echo "TFTP Root: ${R}"

dhcp

tftpboot ${image_addr_r} ${R}/safe-image-fit-1.0-r0-@@MACHINE@@.fitimage

bootm ${image_addr_r}