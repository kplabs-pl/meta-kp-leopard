echo "Running Leopard ebb-leopard-boot-flash.scr"

# Controls from which boot slot image should be loaded.
setenv boot_image 0

echo "Selected BOOT image ${boot_image}"

setenv image_addr_r 0x10000000
setenv image_offset_f 0x400000
setenv image_size 0x3C00000

setenv safe_image "echo Booting from SPI... && sf probe && sf read ${image_addr_r} ${image_offset_f} ${image_size} && bootm ${image_addr_r}"

setenv nand_0_offset 0x0
setenv nand_0_size 0x40000000

setenv nand_1_offset 0x40000000
setenv nand_1_size 0x40000000

setenv nand_2_offset 0x80000000
setenv nand_2_size 0x40000000

setenv boot_nand_configure_mtd "echo Configuring MTD partions && setenv mtdids nand0=nand0 && printenv mtdparts && mtdparts"
setenv boot_nand_mount_ubi "echo Mounting UBI volume && ubi part rootfs 8192 && ubifsmount ubi0:rootfs"
setenv boot_nand_execute_script "echo Loading image-specific boot script && ubifsload ${pxefile_addr_r} /boot/boot.scr && echo Executing image-specific boot script && source ${pxefile_addr_r}"

setenv boot_nand "run boot_nand_configure_mtd && run boot_nand_mount_ubi && run boot_nand_execute_script"

setenv image0 "echo Booting from NAND Image0 && setenv mtdparts nand0:${nand_0_size}@${nand_0_offset}(rootfs) && setenv nand_parition_name image0 && run boot_nand"
setenv image1 "echo Booting from NAND Image1 && setenv mtdparts nand0:${nand_1_size}@${nand_1_offset}(rootfs) && setenv nand_parition_name image1 && run boot_nand"
setenv image2 "echo Booting from NAND Image2 && setenv mtdparts nand0:${nand_2_size}@${nand_2_offset}(rootfs) && setenv nand_parition_name image2 && run boot_nand"

setenv boot_image_0 "run image0"
setenv boot_image_1 "run image1"
setenv boot_image_2 "run image2"
setenv boot_image_3 "run safe_image"
setenv boot_image_4 "run safe_image"
setenv boot_image_5 "run safe_image"
setenv boot_image_6 "run safe_image"
setenv boot_image_7 "run safe_image"

run boot_image_${boot_image}

echo "Image failed to boot. Fallback to safe image"
run safe_image

panic "Safe image also failed. RIP"
