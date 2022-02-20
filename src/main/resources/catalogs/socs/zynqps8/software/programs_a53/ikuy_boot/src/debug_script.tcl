connect -url tcp:127.0.0.1:3121
source /tools/Xilinx/Vitis/2021.2/scripts/vitis/util/zynqmp_utils.tcl
targets


#Change bootmode to jtag and reboot
targets -set -nocase -filter {name =~ "*PSU*"}
stop
mwr  0xff5e0200 0x0100
rst -system

#Disable Security gates to view PMU MB target
targets -set -filter {name =~ "PSU"}
mwr 0xffca0038 0x1ff
after 100

targets -set -nocase -filter {name =~"APU*"}
rst -system
after 100

targets -set -nocase -filter {name =~ "*A53*#0"}
rst -processor
dow ./ikuy_boot
#bpadd main
con -block
