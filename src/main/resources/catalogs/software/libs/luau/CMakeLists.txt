include(ExternalProject)
ExternalProject_Add(luau
        GIT_REPOSITORY https://github.com/Roblox/luau.git
        PREFIX "${CMAKE_SOURCE_DIR}/build/host-luau/luau"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/build/host-lua"
        INSTALL_COMMAND ""
        CMAKE_ARGS
            -DCPU=host
            -DCMAKE_BUILD_TYPE=Release
        INSTALL_COMMAND ""
        )
ExternalProject_Get_property(luau BINARY_DIR)
add_library(luau_compiler STATIC IMPORTED GLOBAL)
add_library(luau_ast STATIC IMPORTED GLOBAL)
add_library(luau_vm STATIC IMPORTED GLOBAL)

set_property(TARGET luau_compiler PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.Compiler.a)
set_property(TARGET luau_ast PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.Ast.a)
set_property(TARGET luau_vm PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.VM.a)

