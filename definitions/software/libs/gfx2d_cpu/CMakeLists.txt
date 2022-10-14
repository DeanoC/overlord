set(project gfx2d_cpu)
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
		include/${project}/*.h
		include/${project}/*.hpp
		include/${project}/${BOARD}/*.h
		include/${project}/${BOARD}/*.hpp
		include/${project}/${SOC}/*.h
		include/${project}/${SOC}/*.hpp
		include/${project}/${CPU_ARCH}/*.h
		include/${project}/${CPU_ARCH}/*.hpp
		include/${project}/${CPU}/*.h
		include/${project}/${CPU}/*.hpp
		)
set(PRIVATE_LIBRARIES
		core
		dbg
		library_defines
		utils
		memory
		tiny_image_format
		image
		)
set(PUBLIC_LIBRARIES
		tiny_image_format
		gfx2d
		)

set( SRCS ${CLIST} ${CPPLIST} ${ASMLIST})
if(DEFINED SRCS)
	list(LENGTH SRCS SRCS_LENGTH)
endif()

if(SRCS_LENGTH)
	add_library(${project} ${SRCS})
	target_include_directories(${project} PUBLIC include)
	target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})
	target_link_libraries(${project} PUBLIC ${PUBLIC_LIBRARIES})
else()
	add_library(${project} INTERFACE)
	target_include_directories(${project} INTERFACE include)
	target_link_libraries(${project} INTERFACE ${PRIVATE_LIBRARIES} ${PUBLIC_LIBRARIES})
endif()

add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
