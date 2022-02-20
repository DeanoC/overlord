set(project utils)
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
		include/utils/*.h
		include/utils/*.hpp
		include/utils/${BOARD}/*.h
		include/utils/${BOARD}/*.hpp
		include/utils/${SOC}/*.h
		include/utils/${SOC}/*.hpp
		include/utils/${CPU_ARCH}/*.h
		include/utils/${CPU_ARCH}/*.hpp
		include/utils/${CPU}/*.h
		include/utils/${CPU}/*.hpp
		)

add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${HEADERLIST})
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE hw)
target_link_libraries(${project} PRIVATE dbg)

target_include_directories(${project} PUBLIC include)
target_compile_definitions(${project} PRIVATE BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)