cmake_minimum_required (VERSION 2.6)

project (asl-test)

ENABLE_TESTING()

find_package( Boost REQUIRED COMPONENTS unit_test_framework filesystem system )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-std=c++11 ${Boost_DEFINITIONS} -DADOBE_STD_SERIALIZATION" )

# Because Carbon/64 has some different Navigation API calls.
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   message( STATUS "OS X: forcing 32 bit build" )
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

# Adam Smoke Test
add_executable (adam_smoke adam_smoke/adam_smoke_test.cpp) 
target_link_libraries( adam_smoke asl ${Boost_LIBRARIES} )
file(COPY adam_smoke DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files" FILES_MATCHING PATTERN "rtd*" )

ADD_TEST(adam_smoke ${CMAKE_CURRENT_BINARY_DIR}/adam_smoke test_files/adam_smoke/rtd)
