set(project library_defines)
project(${project} C)

add_library(${project} INTERFACE)
target_include_directories(${project} INTERFACE include)

add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)



