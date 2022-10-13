set(project pmu_monitor)
project(${project} C ASM CXX)

file(GLOB LINKSCRIPT CONFIGURE_DEPENDS src/lscript.ld)
file(GLOB_RECURSE ASMFILES CONFIGURE_DEPENDS src/*.S)
file(GLOB_RECURSE SRCLIST CONFIGURE_DEPENDS src/*.c src/*.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-M=${project}.map -T ${LINKSCRIPT}")

add_executable(${project} ${SRCLIST} ${ASMFILES} ${LINKSCRIPT})
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)

# minimal memory font for pmu
if(${CPU} STREQUAL "pmu")
	# if changing font size needs needs changin in gfx debug as well
	add_compile_definitions(${project} GFXDEBUG_FONTS_MINIMAL_MEMORY=2)
endif()

target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE gfxdebug)
target_link_libraries(${project} PRIVATE osservices)
target_link_libraries(${project} PRIVATE platform)
target_link_libraries(${project} PRIVATE utils)
target_link_libraries(${project} PRIVATE zynqps8_dma)
target_link_libraries(${project} PRIVATE -lgcc)

add_custom_command(TARGET ${project}
		DEPENDS ${project}
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${CMAKE_CURRENT_BINARY_DIR}/${project} ${CMAKE_CURRENT_BINARY_DIR}/${project}.bin)