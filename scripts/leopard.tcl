package require cmdline

set options {
    {ip.arg             ""          "IP of computer with hw_server running"}
    {project_name.arg   ""          "Project name"}
    {build_path.arg     ""          "Path to build folder"}
    {boot_scr.arg       ""          "boot scr name"}
    {jtag_serial.arg    ""          "JTAG cable serial number"}
}

set usage "xsct leopard.tcl <arguments>"
array set params [::cmdline::getoptions argv $options $usage]

set project_deploy_path "$params(build_path)/tmp/deploy/images/$params(project_name)"

proc proc_wait { timeout }  {
    puts "Waiting $timeout seconds ..."
    after [expr $timeout * 1000]
}

puts "Connecting to host $params(ip)"
connect -host $params(ip)

# Show PMU MicroBlaze on JTAG chain
targets -set -nocase -filter {jtag_cable_serial =~ "$params(jtag_serial)" && name =~ "*PSU*"}
rst -system
mwr 0xFFCA0038 0x1FF
proc_wait 1

# Download and run PMU
targets -set -nocase -filter {jtag_cable_serial =~ "$params(jtag_serial)" && name =~ "MicroBlaze PMU"}

set pmu_firmware_path "$project_deploy_path/pmu-firmware-$params(project_name).elf"
puts "Download PMU firmware: $pmu_firmware_path"
dow $pmu_firmware_path
con
proc_wait 1

# Download and run FSBL
set fsbl_firmware_path "$project_deploy_path/fsbl-$params(project_name).elf"
puts "Download FSBL firmware: $fsbl_firmware_path"
targets -set -nocase -filter {jtag_cable_serial =~ "$params(jtag_serial)" && name =~ "*A53*#0*"}
rst -processor -clear-registers
dow $fsbl_firmware_path
con

# Such a long wait is sometimes required as FSBL was
# throwing "Memory write error at 0x100000;  MMU fault at VA (...)".
# TODO https://kplabs.atlassian.net/browse/LPS-532
proc_wait 30

# Download u-boot
set u_boot_path "$project_deploy_path/u-boot-$params(project_name).elf"
dow -data "$project_deploy_path/system.dtb" 0x100000
puts "Download u-boot: $u_boot_path"
dow $u_boot_path

# Download boot.scr
set boot_scr_path "$project_deploy_path/u-boot-scripts/boot-scripts/$params(boot_scr)"
puts "Download boot.scr: $boot_scr_path"
dow -data $boot_scr_path 0x20000000

# Download atf
set atf_path $project_deploy_path/arm-trusted-firmware.elf
puts "Download ATF: $atf_path"
dow $atf_path

puts "Finished"

con