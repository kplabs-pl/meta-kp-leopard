package require cmdline

set options {
    {ip.arg             ""          "IP of computer with hw_server running"}
    {boot_mode.arg      ""          "Boot mode"}
}

set usage "xsct boot_mode.tcl <arguments>"
array set params [::cmdline::getoptions argv $options $usage]

puts "Connecting to host $params(ip)"
connect -host $params(ip) -port 3121

set jtagBootMode 0x0100 
set qspi32BootMode 0x2100  
set sdBootMode 0x5100  

# Show PMU MicroBlaze on JTAG chain
targets -set -nocase -filter {name =~ "*PSU*"}

if {$params(boot_mode)=="jtag"} {
    puts "Setting JTAG boot mode"
    rwr crl_apb boot_mode_user $jtagBootMode
} elseif {$params(boot_mode)=="qspi32"} {
    puts "Setting QSPI32 boot mode"
    rwr crl_apb boot_mode_user $qspi32BootMode
} elseif {$params(boot_mode)=="sd"} { 
    puts "Setting SD card boot mode"
    rwr crl_apb boot_mode_user $sdBootMode
} else {
    puts "$params(boot_mode) is not correct boot mode"
    puts "Available boot modes are: jtag, qspi32, sd"
}