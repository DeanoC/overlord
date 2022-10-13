set(project data_binify)
project(${project} C ASM CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${PARSER_DIR}/scanner.cpp")
set(PARSER_OUT "${PARSER_DIR}/parser.cpp")

flex_target(LEXER "src/lexer.ll" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/scanner.hpp")
bison_target(PARSER "src/binify.yy" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.hpp")
add_flex_bison_dependency(LEXER PARSER)

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
		cadt
		core
		dbg
		library_defines
		memory
		multi_core
		utils
		vfile
		tiny_stl
		)


add_library(${project} ${CLIST} ${CPPLIST} ${ASMLIST} "${LEXER_OUT}" "${PARSER_OUT}")
target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})

target_include_directories(${project} PUBLIC include)
target_include_directories(${project} PRIVATE src)
target_include_directories(${project} PRIVATE ${PARSER_DIR})
target_compile_definitions(${project} PRIVATE BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)

