connect -url tcp:127.0.0.1:3121
targets
source /home/deano/fpga_drive/Xilinx/Vitis/2021.1/scripts/vitis/util/zynqmp_utils.tcl

#Disable Security gates to view PMU MB target
targets -set -filter {name =~ "PSU"}
mwr 0xffca0038 0x1ff
after 100

targets -set -nocase -filter {name =~"APU*"}
rst -system
after 100

targets -set -nocase -filter {name =~ "*A53*#0"}
rst -processor
dow ./build/programs_a53-debug/programs_a53/ikuy_boot/ikuy_boot
#bpadd main
con -block
