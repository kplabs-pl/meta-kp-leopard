# ZynqMP DRAM Test

This recipe produces an executable used for validating DDR timings.
It can be also built using Vitis IDE, as described in this article: https://adaptivesupport.amd.com/s/article/1216727?language=en_US

## Building

`bitbake zynqmp-dram-test`

## Running

This recipe comes with `zynqmp-dram-test.tcl` script used for flashing Leopard with PMU firmware, FSBL firmware and DRAM test itself.
Just run the following:

`xsct ./zynqmp-dram-test.tcl -artifacts_path <path_to_built_artifacts> -machine leopard-<ebb|dpu>`

Note: System must be in JTAG boot mode!

### Expected output from flashing:
```
WARNING: dbus-launch is not available on the system, please make sure dbus-launch is available on the system.
Display is :113
WARNING: sdtgen package cannot be loaded. System Device tree commands will not
be available
Waiting 1 seconds ...
Download PMU firmware: ./pmu-firmware-leopard-dpu.elf
Waiting 1 seconds ...
Download FSBL firmware: ./fsbl-leopard-dpu.elf
Waiting 30 seconds ...
Download ZynqMP DRAM Test: ./zynqmp-dram-test-leopard-dpu.elf
Finished
```

### Expected output from Leopard's debug term:
```
Zynq MP First Stage Boot Loader
Release 2025.1   Aug 25 2025  -  16:33:53
[SI5338] I2C initialized
[SI5338] Clock generator configured successfully
[SI5338] done!

****************************************************************************
   Zynq MPSoC
   DRAM Diagnostics Test (A53)
****************************************************************************
   Select one of the options below:
   +--------------------------------------------------------------------+
   |  Memory Tests                                                      |
   +-----+--------------------------------------------------------------+
   | '0' | Test first 16MB region of DDR                                |
   | '1' | Test first 32MB region of DDR                                |
   | '2' | Test first 64MB region of DDR                                |
   | '3' | Test first 128MB region of DDR                               |
   | '4' | Test first 256MB region of DDR                               |
   | '5' | Test first 512MB region of DDR                               |
   | '6' | Test first 1GB region of DDR                                 |
   | '7' | Test first 2GB region of DDR                                 |
   | '8' | Test first 4GB region of DDR                                 |
   | '9' | Test first 8GB region of DDR                                 |
   | 'm' | Test user specified size in MB of DDR                        |
   | 'g' | Test user specified size in GB of DDR                        |
   +-----+--------------------------------------------------------------+
   |  Eye Tests                                                         |
   +-----+--------------------------------------------------------------+
   | 'r' | Perform a read eye analysis test                             |
   | 'w' | Perform a write eye analysis test                            |
   | 'c' | Perform a 2-D read eye analysis test                         |
   | 'e' | Perform a 2-D write eye analysis test                        |
   | 'a' | Print test start address                                     |
   | 'l' | Select Number of Iterations for Memory/Read/Write-Eye/2D test|
   | 't' | Specify test start address (default=0x0)                     |
   | 's' | Select the DRAM Rank (default=0)                             |
   +-----+--------------------------------------------------------------+
   |  Miscellaneous options                                             |
   +-----+--------------------------------------------------------------+
   | 'i' | Print DDR information                                        |
   | 'v' | Verbose Mode ON/OFF                                          |
   | 'o' | Toggle cache enable/disable                                  |
   | 'b' | Toggle between 16/32/64-bit bus widths                       |
   | 'q' | Exit the DRAM Test                                           |
   | 'h' | Print this help menu                                         |
   +-----+--------------------------------------------------------------+

        Bus Width = 64,   D-cache is disable,   Verbose Mode is OFF,   DDR ECC is ENABLED

 Enter 'h' to print help menu
 Enter Test Option:
```