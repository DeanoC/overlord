set(project image_convert)
project(${project} C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB SRCLIST CONFIGURE_DEPENDS src/*.c src/*.cpp)

set(PRIVATE_LIBRARIES
			core
			library_defines
			dbg
			memory
			cadt
			utils
			multi_core
			vfile
			vfile_memory
			tiny_image_format
			image
			host_os
			luau_compiler
			luau_ast
			luau_hvm
			luau_utils
			data_binify
			data_utils
			tiny_stl
			resource_bundle
			resource_bundle_writer
			gfx2d
            gfx2d_cpu
        )

add_executable(${project} ${SRCLIST})
target_link_libraries(${project} PRIVATE ${PRIVATE_LIBRARIES})
target_link_libraries(${project} PRIVATE pthread)

add_compile_definitions(${project} BOARD_${BOARD}=1 SOC_${SOC}=1 CPU_${CPU}=1 CPU_ARCH_${CPU_ARCH}=1)
