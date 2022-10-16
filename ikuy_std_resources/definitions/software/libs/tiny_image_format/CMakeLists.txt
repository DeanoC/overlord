set(project tiny_image_format)
project(${project} C ASM)

set(CMAKE_C_STANDARD 11)

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
		include/*.h
		include/${BOARD}/*.h
		include/${SOC}/*.h
		include/${CPU_ARCH}/*.h
		include/${CPU}/*.h
		)

set( SRCS ${CLIST} ${ASMLIST})
if(DEFINED SRCS)
	list(LENGTH SRCS SRCS_LENGTH)
endif()

set(PRIVATE_LIBRARIES
		core
		platform
		)

if(NOT ${CPU} STREQUAL "host")
	list(APPEND PRIVATE_LIBRARIES musl_math)
endif()

if(SRCS_LENGTH)
	add_library(${project} ${SRCS} ${HEADERLIST})
	target_include_directories(${project} PUBLIC include)
	target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})
else()
	add_library(${project} INTERFACE ${HEADERLIST} )
	target_include_directories(${project} INTERFACE include)
	target_link_libraries(${project} INTERFACE ${PRIVATE_LIBRARIES})
endif()

add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
