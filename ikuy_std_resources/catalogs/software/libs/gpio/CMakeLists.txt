set(project gpio)
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

file(GLOB CHEADERLIST CONFIGURE_DEPENDS
		include/*.h
		include/${BOARD}/*.h
		include/${SOC}/*.h
		include/${CPU_ARCH}/*.h
		include/${CPU}/*.h
		)

file(GLOB CPPHEADERLIST CONFIGURE_DEPENDS
		include/*.hpp
		include/${BOARD}/*.hpp
		include/${SOC}/*.hpp
		include/${CPU_ARCH}/*.hpp
		include/${CPU}/*.hpp
		)
add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${CHEADERLIST} ${CPPHEADERLIST})

target_include_directories(${project} PUBLIC include)
target_compile_definitions(${project} PRIVATE BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE library_defines)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE platform)
