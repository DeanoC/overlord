set(project data_binify)
project(${project} C ASM CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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

add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST})
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE memory)
target_link_libraries(${project} PRIVATE multi_core)
target_link_libraries(${project} PRIVATE utils)
target_link_libraries(${project} PRIVATE vfile)
target_link_libraries(${project} PRIVATE cadt)
target_link_libraries(${project} PRIVATE tiny_stl)

target_include_directories(${project} PUBLIC include)
target_compile_definitions(${project} PRIVATE BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)

