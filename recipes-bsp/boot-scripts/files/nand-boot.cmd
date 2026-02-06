echo "Running Leopard nand-boot.scr"

echo "Loading boot files"
ubifsload ${kernel_addr_r} /boot/Image
ubifsload ${fdt_addr_r} /boot/devicetree/system-top.dtb

echo "Booting"
setenv bootargs "$bootargs rootfstype=ubifs root=ubi0:rootfs ubi.mtd=${nand_parition_name},8192"
booti ${kernel_addr_r} - ${fdt_addr_r}
