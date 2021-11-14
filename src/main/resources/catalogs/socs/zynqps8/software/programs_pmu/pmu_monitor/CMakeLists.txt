set(project pmu_monitor)
project(${project} C ASM CXX)

file(GLOB LINKSCRIPT CONFIGURE_DEPENDS src/lscript.ld)
file(GLOB ASMFILES CONFIGURE_DEPENDS src/*.S)
file(GLOB SRCLIST CONFIGURE_DEPENDS src/*.c src/*.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-M=${project}.map -T ${LINKSCRIPT}")

add_executable(${project} ${SRCLIST} ${ASMFILES} ${LINKSCRIPT})
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE utils)
target_link_libraries(${project} PRIVATE hw)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE osservices)
target_link_libraries(${project} PRIVATE zynqps8_dma)
target_link_libraries(${project} PRIVATE gfxdebug)
target_link_libraries(${project} PRIVATE -lgcc)

add_custom_command(TARGET ${project}
		DEPENDS ${project}
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${CMAKE_CURRENT_BINARY_DIR}/${project} ${CMAKE_CURRENT_BINARY_DIR}/${project}.bin)