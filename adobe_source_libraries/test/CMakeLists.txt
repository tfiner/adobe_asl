cmake_minimum_required (VERSION 2.6)

project (asl-test)

ENABLE_TESTING()

find_package( Boost REQUIRED COMPONENTS unit_test_framework filesystem system )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-std=c++11 ${Boost_DEFINITIONS} -DADOBE_STD_SERIALIZATION -DBOOST_TEST_DYN_LINK" )

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

# Adam Tutorial it's interactive and not really a test.
# It expects the input.cel and default.adm in it's cwd or it aborts.
add_executable (adam_tutorial adam_tutorial/main.cpp) 
target_link_libraries( adam_tutorial asl ${Boost_LIBRARIES} )
file(COPY adam_tutorial/input.cel adam_tutorial/default.adm DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files/adam_tutorial" )
#ADD_TEST(
#    NAME adam_tutorial 
#    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_files/adam_tutorial"
#    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/adam_tutorial)

set(TESTS any_iterator closed_hash cmath dictionary_benchmark)
foreach(TEST ${TESTS})
    add_executable( ${TEST} ${TEST}/main.cpp) 
    target_link_libraries( ${TEST} asl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach()

# Eve Smoke Test
add_executable (eve_smoke eve_smoke/main.cpp) 
target_link_libraries( eve_smoke asl ${Boost_LIBRARIES} )
file(COPY eve_smoke/default.eve DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files/eve_smoke" )
ADD_TEST(
    NAME eve_smoke 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_files/eve_smoke
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/eve_smoke
)

