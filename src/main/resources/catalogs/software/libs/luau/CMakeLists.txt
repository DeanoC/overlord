
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(ExternalProject)
ExternalProject_Add(luau
        GIT_REPOSITORY https://github.com/Roblox/luau.git
        PREFIX "${CMAKE_SOURCE_DIR}/build/host-luau"
        INSTALL_COMMAND ""
        CMAKE_ARGS
        -DCPU=host
        -DCMAKE_BUILD_TYPE=Release
        )
add_library(luau_compiler STATIC IMPORTED GLOBAL)
add_library(luau_ast STATIC IMPORTED GLOBAL)
add_library(luau_hvm STATIC IMPORTED GLOBAL)
add_dependencies(luau_compiler PRIVATE luau)
ExternalProject_Get_property(luau BINARY_DIR)
ExternalProject_Get_property(luau SOURCE_DIR)

set_property(TARGET luau_compiler PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.Compiler.a)
target_include_directories(luau_compiler INTERFACE ${SOURCE_DIR}/Compiler/include)
target_include_directories(luau_compiler INTERFACE ${SOURCE_DIR}/Ast/include)

set_property(TARGET luau_ast PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.Ast.a)
target_include_directories(luau_ast INTERFACE ${SOURCE_DIR}/Compiler/include)
target_include_directories(luau_ast INTERFACE ${SOURCE_DIR}/Ast/include)

set_property(TARGET luau_hvm PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libLuau.VM.a)
target_include_directories(luau_hvm INTERFACE ${SOURCE_DIR}/VM/include)
target_include_directories(luau_hvm INTERFACE ${SOURCE_DIR}/Compiler/include)
target_include_directories(luau_hvm INTERFACE ${SOURCE_DIR}/Ast/include)

