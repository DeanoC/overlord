set(project ikuy_boot)
project(${project} C ASM)

file(GLOB LINKSCRIPT CONFIGURE_DEPENDS lscript.ld)
file(GLOB ASMFILES CONFIGURE_DEPENDS *.S)
file(GLOB SRCLIST CONFIGURE_DEPENDS *.c)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-M=${project}.map -T ${LINKSCRIPT}")

add_executable(${project} ${SRCLIST} ${ASMFILES} ${LINKSCRIPT})
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_${CPU_ARCH}=1)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE hw)
target_link_libraries(${project} PRIVATE dbg)
#target_link_libraries(${project} PRIVATE latent_functions)