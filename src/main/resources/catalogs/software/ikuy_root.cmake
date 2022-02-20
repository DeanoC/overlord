cmake_minimum_required(VERSION 3.18)

# To help IntelliJ parser
if(NOT DEFINED CPU)
	set(CPU "a53")
endif()
if(NOT DEFINED BOARD)
	set(BOARD "None")
endif()
if(NOT DEFINED SOC)
	set(SOC "None")
endif()

#TODO move this part out
if(${BOARD} STREQUAL "kv260" )
	set(SOC "zynqmp")
endif()

if(${BOARD} STREQUAL "myirfz3" )
	set(SOC "zynqmp")
endif()

project(ikuy_sw_${CPU} C ASM)

if(${CPU} STREQUAL "a53")
	set(CPU_ARCH "aarch64")
elseif(${CPU} STREQUAL "pmu")
	set(CPU_ARCH "microblaze")
endif()

add_subdirectory(libs)
if(CPU STREQUAL "host")
	add_subdirectory(libs_host)
else()
	add_subdirectory(libs_target)
endif()

add_subdirectory(programs_${CPU})