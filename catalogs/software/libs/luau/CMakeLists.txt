add_subdirectory(src)

add_library(luau_compiler STATIC IMPORTED GLOBAL)
add_library(luau_ast STATIC IMPORTED GLOBAL)
add_library(luau_hvm STATIC IMPORTED GLOBAL)
add_dependencies(luau_compiler PRIVATE luau)
set_property(TARGET luau_compiler PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/src/libLuau.Compiler.a)
target_include_directories(luau_compiler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Compiler/include)
target_include_directories(luau_compiler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Ast/include)

set_property(TARGET luau_ast PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/src/libLuau.Ast.a)
target_include_directories(luau_ast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Compiler/include)
target_include_directories(luau_ast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Ast/include)

set_property(TARGET luau_hvm PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/src/libLuau.VM.a)
target_include_directories(luau_hvm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/VM/include)
target_include_directories(luau_hvm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Compiler/include)
target_include_directories(luau_hvm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/Ast/include)



