set(project luau_utils)
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
		include/${projects}/*.h
		include/${projects}/${BOARD}/*.h
		include/${projects}/${SOC}/*.h
		include/${projects}/${CPU_ARCH}/*.h
		include/${projects}/${CPU}/*.h
		)

file(GLOB CPPHEADERLIST CONFIGURE_DEPENDS
		include/${projects}/*.hpp
		include/${projects}/${BOARD}/*.hpp
		include/${projects}/${SOC}/*.hpp
		include/${projects}/${CPU_ARCH}/*.hpp
		include/${projects}/${CPU}/*.hpp
		)
set(PRIVATE_LIBRARIES
		core
		dbg
		)
set(PUBLIC_LIBRARIES
		)
if(${CPU} STREQUAL "host")
	list(APPEND PRIVATE_LIBRARIES luau_hvm )
else()
	list(APPEND PRIVATE_LIBRARIES luau_vm)
endif()

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

target_compile_definitions(${project} PRIVATE BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)