set(project zynqps8_i2c)
project(${project} C ASM CXX)

# requires parent cmake project to set BOARD_ SOC_ CPU_ and CPU_ARCH
file(GLOB CLIST CONFIGURE_DEPENDS
		src/*.c
		src/${BOARD}/*.c
		src/${SOC}/*.c
		src/${CPU_ARCH}/*.c
		src/${CPU}/*.c )

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

file(GLOB HEADERLIST CONFIGURE_DEPENDS
		include/zynqps8/i2c/*.h
		include/zynqps8/i2c/*.hpp
		include/zynqps8/i2c/${BOARD}/*.h
		include/zynqps8/i2c/${BOARD}/*.hpp
		include/zynqps8/i2c/${SOC}/*.h
		include/zynqps8/i2c/${SOC}/*.hpp
		include/zynqps8/i2c/${CPU_ARCH}/*.h
		include/zynqps8/i2c/${CPU_ARCH}/*.hpp
		include/zynqps8/i2c/${CPU}/*.h
		include/zynqps8/i2c/${CPU}/*.hpp
		)
set(PRIVATE_LIBRARIES
		core
		library_defines
		dbg
		multi_core
		platform
		utils
		zynqps8_gic_v2
		xilinx
		)

add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${HEADERLIST})
target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})
target_include_directories(${project} PUBLIC include)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
