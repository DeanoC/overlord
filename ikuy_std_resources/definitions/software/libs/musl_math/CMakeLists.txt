set(project musl_math)
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
		include/zynqps8/dma/*.h
		include/zynqps8/dma/*.hpp
		include/zynqps8/dma/${BOARD}/*.h
		include/zynqps8/dma/${BOARD}/*.hpp
		include/zynqps8/dma/${SOC}/*.h
		include/zynqps8/dma/${SOC}/*.hpp
		include/zynqps8/dma/${CPU_ARCH}/*.h
		include/zynqps8/dma/${CPU_ARCH}/*.hpp
		include/zynqps8/dma/${CPU}/*.h
		include/zynqps8/dma/${CPU}/*.hpp
		)

add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${HEADERLIST})
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE osservices)
target_link_libraries(${project} PRIVATE platform)

target_include_directories(${project} PUBLIC include)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)