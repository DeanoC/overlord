set(project gfxdebug)
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
		include/gfxdebug/*.h
		include/gfxdebug/*.hpp
		include/gfxdebug/${BOARD}/*.h
		include/gfxdebug/${BOARD}/*.hpp
		include/gfxdebug/${SOC}/*.h
		include/gfxdebug/${SOC}/*.hpp
		include/gfxdebug/${CPU_ARCH}/*.h
		include/gfxdebug/${CPU_ARCH}/*.hpp
		include/gfxdebug/${CPU}/*.h
		include/gfxdebug/${CPU}/*.hpp
		)

add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} ${HEADERLIST})
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE dbg)
target_link_libraries(${project} PRIVATE utils)


target_include_directories(${project} PUBLIC include)
add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
if(${CPU} STREQUAL "pmu")
	# if changing font size needs needs changin in pmu monitor as well
	add_compile_definitions(${project} GFXDEBUG_FONTS_MINIMAL_MEMORY=2)
endif()
