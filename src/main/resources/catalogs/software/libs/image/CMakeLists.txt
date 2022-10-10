set(project image)
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

set(PRIVATE_LIBRARIES
		core
		library_defines
		dbg
		memory
    	multi_core
		platform
		tiny_image_format
		)

if(${IKUY_HAVE_LIB_LUAU})
	list(APPEND PRIVATE_LIBRARIES luau_hvm luau_utils)
	if(${IKUY_HAVE_LIB_IMAGE_IO})
		list(APPEND PRIVATE_LIBRARIES image_io)
	endif()
endif()
if(${IKUY_HAVE_LIB_LUAU_VM})
	list(APPEND PRIVATE_LIBRARIES luau_vm luau_utils)
endif()

if(NOT ${CPU} STREQUAL "host")
	list(APPEND PRIVATE_LIBRARIES musl_math)
else()
	list(APPEND PRIVATE_LIBRARIES
		vfile
		utils
		data_binify
		tiny_stl
		resource_bundle
		resource_bundle_writer
		host_os)
endif()

set( SRCS ${CLIST} ${CPPLIST} ${ASMLIST})
if(DEFINED SRCS)
	list(LENGTH SRCS SRCS_LENGTH)
endif()

if(SRCS_LENGTH)
	add_library(${project} ${SRCS})
	target_include_directories(${project} PUBLIC include)
	target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})
else()
	add_library(${project} INTERFACE)
	target_include_directories(${project} INTERFACE include)
	target_link_libraries(${project} INTERFACE ${PRIVATE_LIBRARIES})
endif()

add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
