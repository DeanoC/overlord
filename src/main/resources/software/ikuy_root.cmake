cmake_minimum_required(VERSION 3.18)
project(ikuy_sw_${CPU} C ASM)

set(BOARD "myir_fz3")
set(SOC "zynqmp")

if(${CPU} STREQUAL "a53")
	set(CPU_ARCH "aarch64")
elseif(${CPU} STREQUAL "pmu")
	set(CPU_ARCH "microblaze")
endif()

add_subdirectory(libraries)
add_subdirectory(programs_${CPU})