package require cmdline

set options {
    {artifacts_path.arg     ""          "Path to folder with artifacts"}
    {machine.arg            ""          "Machine name"}
}

set usage "xsct leopard.tcl <arguments>"
array set params [::cmdline::getoptions argv $options $usage]

set project_deploy_path "$params(artifacts_path)"

proc proc_wait { timeout }  {
    puts "Waiting $timeout seconds ..."
    after [expr $timeout * 1000]
}

connect

# Show PMU MicroBlaze on JTAG chain
targets -set -nocase -filter {name =~ "*PSU*"}
rst -system
mwr 0xFFCA0038 0x1FF
proc_wait 1

# Download and run PMU
targets -set -nocase -filter {name =~ "MicroBlaze PMU"}

set pmu_firmware_path "$project_deploy_path/pmu-firmware-leopard-dpu.elf"
puts "Download PMU firmware: $pmu_firmware_path"
dow $pmu_firmware_path
con
proc_wait 1

# Download and run FSBL
set fsbl_firmware_path "$project_deploy_path/fsbl-leopard-dpu.elf"
puts "Download FSBL firmware: $fsbl_firmware_path"
targets -set -nocase -filter {name =~ "*A53*#0*"}
rst -processor -clear-registers
dow $fsbl_firmware_path
con

# Such a long wait is sometimes required as FSBL was
# throwing "Memory write error at 0x100000;  MMU fault at VA (...)".
# TODO https://kplabs.atlassian.net/browse/LPS-532
proc_wait 30

# Download zynqmp_dram_test
set tester_path "$project_deploy_path/zynqmp-dram-test-$params(machine).elf"
puts "Download ZynqMP DRAM Test: $tester_path"
dow $tester_path
con

puts "Finished"
