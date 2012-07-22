cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(adobe)

ENABLE_TESTING()
message( STATUS "CMake detected OS '${CMAKE_SYSTEM_NAME}'" )
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")


# Used to include FindTBB.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Not every sub project needs TBB, but this just finds and sets up the
# variables that can then be referenced by deeper builds.
find_package( TBB )

set(CMAKE_CXX_FLAGS 
    "-std=c++11 -DADOBE_STD_SERIALIZATION -DBOOST_TEST_DYN_LINK" )

# Because Carbon/64 has some different Navigation API calls.
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   message( STATUS "OS X: forcing 32 bit build" )
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wno-deprecated-declarations -isysroot /Developer/SDKs/MacOSX10.6.sdk")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wno-deprecated-declarations")
   set(CMAKE_SHARED_LINKER_FLAGS 
        "-framework AppKit -framework Carbon -framework ApplicationServices -framework AudioToolbox")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DTBB_USE_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

# Make things easier in the deeper projects.
set(ASL_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASL_SRC_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/adobe_source_libraries)
set(ASL_PLAT_REL ${CMAKE_CURRENT_SOURCE_DIR}/platform_release)

# Builds each library separately.
add_subdirectory(${ASL_SRC_LIBS})
add_subdirectory(${ASL_SRC_LIBS}/test)
add_subdirectory(${ASL_SRC_LIBS}/test/unit_tests)
add_subdirectory(${ASL_PLAT_REL})
add_subdirectory(${ASL_PLAT_REL}/test)

# Individual OS X apps have all kinds of their own customizations, and 
#their own CMakeLists.txt
add_subdirectory(${ASL_PLAT_REL}/test/modal_dialog_interface)
