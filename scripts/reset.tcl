package require cmdline

set options {
    {ip.arg             ""          "IP of computer with hw_server running"}
}

set usage "xsct boot_mode.tcl <arguments>"
array set params [::cmdline::getoptions argv $options $usage]

puts "Connecting to host $params(ip)"
connect -host $params(ip) -port 3121

# Prevents halt of Cortex-A53 cores after system reset
configparams disable-access 1

targets -set -nocase -filter {name =~ "*PSU (Access Disabled)*"}
rst -system

configparams disable-access 0
puts "Reset finished"