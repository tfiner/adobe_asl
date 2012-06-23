cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(adobe)

add_subdirectory(adobe_source_libraries)
add_subdirectory(platform_release)

# target_link_libraries(lx server)