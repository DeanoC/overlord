set(project bin2c)
project(${project} C)

add_executable(${project} src/bin2c.c)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
