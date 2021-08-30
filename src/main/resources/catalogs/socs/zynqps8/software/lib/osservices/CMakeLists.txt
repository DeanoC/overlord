set(project osservices)
project(${project} C ASM)

# requires parent cmake project to set BOARD_ SOC_ CPU_ and CPU_ARCH
file(GLOB CLIST CONFIGURE_DEPENDS
		src/*.c
		src/${BOARD}/*.c
		src/${SOC}/*.c
		src/${CPU_ARCH}/*.c
		src/${CPU}/*.c )

file(GLOB ASMLIST CONFIGURE_DEPENDS
		src/*.S
		src/${BOARD}/*.S
		src/${SOC}/*.S
		src/${CPU_ARCH}/*.S
		src/${CPU}/*.S )

file(GLOB HEADERLIST CONFIGURE_DEPENDS
		include/hw/*.h
		include/hw/${BOARD}/*.h
		include/hw/${SOC}/*.h
		include/hw/${CPU_ARCH}/*.h
		include/hw/${CPU}/*.h
		include/hw_regs/*.h
		include/hw_regs/${BOARD}/*.h
		include/hw_regs/${SOC}/*.h
		include/hw_regs/${CPU_ARCH}/*.h
		include/hw_regs/${CPU}/*.h
		)

add_library(${project} ${CLIST} ${ASMLIST} ${HEADERLIST})

target_include_directories(${project} PUBLIC include)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE hw)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)