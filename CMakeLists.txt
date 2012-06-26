cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(adobe)

ENABLE_TESTING()
message( STATUS "CMake detected OS '${CMAKE_SYSTEM_NAME}'" )
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

# Make things easier inthe deeper projects.
set(ASL_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASL_SRC_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/adobe_source_libraries)
set(ASL_PLAT_REL ${CMAKE_CURRENT_SOURCE_DIR}/platform_release)

# Builds each library separately.
add_subdirectory(${ASL_SRC_LIBS})
add_subdirectory(${ASL_SRC_LIBS}/test)
add_subdirectory(${ASL_SRC_LIBS}/test/unit_tests)
add_subdirectory(${ASL_PLAT_REL})
add_subdirectory(${ASL_PLAT_REL}/test)
