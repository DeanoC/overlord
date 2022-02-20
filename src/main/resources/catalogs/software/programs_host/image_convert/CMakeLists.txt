set(project image_convert)
project(${project} C CXX)


add_executable(${project} src/image_convert.c)
target_link_libraries(${project} PRIVATE core)
target_link_libraries(${project} PRIVATE memory)
target_link_libraries(${project} PRIVATE utils)
#target_link_libraries(${project} PRIVATE multi_core)
