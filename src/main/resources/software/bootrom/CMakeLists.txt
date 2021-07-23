cmake_minimum_required(VERSION 3.12.4)

set(project ${boot_cpu}_boot)
project(${project} C ASM)

set(PLATFORM "${boot_cpu}")
set(CPU_ARCH "${triple}")
set(IKUY_PATH ../.. )

file(GLOB LINKSCRIPT CONFIGURE_DEPENDS boot.ld)
file(GLOB ASMFILES CONFIGURE_DEPENDS *.S)
file(GLOB SRCLIST CONFIGURE_DEPENDS *.c )

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-M=${project}.map -T ${LINKSCRIPT}")

add_executable(${project}.elf ${SRCLIST} ${ASMFILES} ${LINKSCRIPT})

add_custom_target(
		${project} ALL
    COMMAND ${CMAKE_OBJCOPY} -Oihex
            ${CMAKE_CURRENT_BINARY_DIR}/${project}.elf
            ${CMAKE_CURRENT_BINARY_DIR}/${project}.hex
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${project}.hex
    DEPENDS ${project}.elf
    )
