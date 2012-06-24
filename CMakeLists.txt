cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(adobe)

ENABLE_TESTING()
message( STATUS "CMake detected OS '${CMAKE_SYSTEM_NAME}'" )
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

# Builds each library separately.
add_subdirectory(adobe_source_libraries)
add_subdirectory(adobe_source_libraries/test)
add_subdirectory(platform_release)
