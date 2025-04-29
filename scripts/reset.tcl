package require cmdline

set options {
    {ip.arg             ""          "IP of computer with hw_server running"}
    {jtag_serial.arg    ""          "JTAG cable serial number"}
}

proc proc_wait { timeout }  {
    after [expr $timeout * 1000]
}

set usage "xsct reset.tcl <arguments>"
array set params [::cmdline::getoptions argv $options $usage]

puts "Connecting to host $params(ip)"
connect -host $params(ip)

# Prevents halt of Cortex-A53 cores after system reset
configparams disable-access 1

# in case qspi32 is chosen `configparams disable-access 1` will not disable access as expected - waiting a little bit is required
proc_wait 3

targets -set -nocase -filter {jtag_cable_serial =~ "$params(jtag_serial)" && name =~ "*PSU (Access Disabled)*"}
rst -system

proc_wait 1
configparams disable-access 0

puts "Reset finished"