set(project xilinx)
project(${project} C ASM CXX)

# requires parent cmake project to set BOARD_ SOC_ CPU_ and CPU_ARCH
file(GLOB CLIST CONFIGURE_DEPENDS
		src/scugic/*.c
		src/sdps/*.c
		src/ttcps/*.c
		src/iicps/*.c
		src/emacps/*.c

		src/${BOARD}/*.c
		src/${SOC}/*.c
		src/${CPU_ARCH}/*.c
		src/${CPU}/*.c )

file(GLOB HEADERLIST CONFIGURE_DEPENDS
		include/scugic/*.h
		include/sdps/*.h
		include/ttcps/*.h
		src/iicps/*.h
		include/emacps/*.h

		include/*.h
		include/*.hpp
		include/${BOARD}/*.h
		include/${BOARD}/*.hpp
		include/${SOC}/*.h
		include/${SOC}/*.hpp
		include/${CPU_ARCH}/*.h
		include/${CPU_ARCH}/*.hpp
		include/${CPU}/*.h
		include/${CPU}/*.hpp
		)

file(GLOB CPPLIST CONFIGURE_DEPENDS
		src/*.cpp
		src/${BOARD}/*.cpp
		src/${SOC}/*.cpp
		src/${CPU_ARCH}/*.cpp
		src/${CPU}/*.cpp )

file(GLOB ASMLIST CONFIGURE_DEPENDS
		src/*.S
		src/${BOARD}/*.S
		src/${SOC}/*.S
		src/${CPU_ARCH}/*.S
		src/${CPU}/*.S )


add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${HEADERLIST})
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE utils)


target_include_directories(${project} PUBLIC include)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
