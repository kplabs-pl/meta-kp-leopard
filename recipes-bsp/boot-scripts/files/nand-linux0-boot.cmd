echo "Running Leopard nand-linux0-boot.scr"

setenv nand_0_offset 0x0
setenv nand_0_size 0x40000000

setenv boot_nand_configure_mtd "echo Configuring MTD partions && setenv mtdids nand0=nand0 && printenv mtdparts && mtdparts"
setenv boot_nand_mount_ubi "echo Mounting UBI volume && ubi part rootfs 8192 && ubifsmount ubi0:rootfs"
setenv boot_nand_execute_script "echo Loading image-specific boot script && ubifsload ${pxefile_addr_r} /boot/boot.scr && echo Executing image-specific boot script && source ${pxefile_addr_r}"

setenv boot_nand "run boot_nand_configure_mtd && run boot_nand_mount_ubi && run boot_nand_execute_script"

setenv linux0 "echo Booting from NAND Linux0 && setenv mtdparts nand0:${nand_0_size}@${nand_0_offset}(rootfs) && setenv nand_parition_name linux0 && run boot_nand"

run linux0