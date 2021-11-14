set(project ikuy_boot)
project(${project} C CXX ASM)
include(ExternalProject)

ExternalProject_Add(pmu_monitor
		PREFIX "${CMAKE_SOURCE_DIR}/build/pmu_build"
		SOURCE_DIR "${CMAKE_SOURCE_DIR}"
		INSTALL_COMMAND ""
		CMAKE_ARGS
			-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/pmu_toolchain.cmake
			-DCPU=pmu
		)
ExternalProject_Get_Property(pmu_monitor binary_dir)

file(GLOB LINKSCRIPT CONFIGURE_DEPENDS src/lscript.ld)
file(GLOB ASMFILES CONFIGURE_DEPENDS src/*.S)
file(GLOB SRCLIST CONFIGURE_DEPENDS src/*.c src/*.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-M=${project}.map -T ${LINKSCRIPT}")

set(PROJECT_EMBED_OBJ_FILES "")
set(PROJECT_EMBED_FILES "${binary_dir}/programs_pmu/pmu_monitor/pmu_monitor.bin")
foreach(FILENAME ${PROJECT_EMBED_FILES})
	get_filename_component(FILENAME_ONLY ${FILENAME} NAME)
	get_filename_component(FILEPATH_ONLY ${FILENAME} PATH)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o
			WORKING_DIRECTORY ${FILEPATH_ONLY}
			COMMAND ${CMAKE_OBJCOPY}
			ARGS -I binary -O elf64-littleaarch64 --rename-section .data=.programs ${FILENAME_ONLY} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o )
	list(APPEND PROJECT_EMBED_OBJ_FILES ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_ONLY}.o)
	message(STATUS "Objcopy'ing ${FILENAME}")
endforeach(FILENAME)


add_executable(${project} ${SRCLIST} ${ASMFILES} ${LINKSCRIPT} ${PROJECT_EMBED_OBJ_FILES})
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_${CPU_ARCH}=1)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE utils)
target_link_libraries(${project} PRIVATE hw)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE osservices)
target_link_libraries(${project} PRIVATE zynqps8_display_port)
target_link_libraries(${project} PRIVATE gfxdebug)

add_dependencies(${project} pmu_monitor)
