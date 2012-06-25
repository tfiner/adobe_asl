cmake_minimum_required (VERSION 2.6)

# To see what failed in a test, set the environment variable
# CTEST_OUTPUT_ON_FAILURE
project (asl-unit-test)

find_package( Boost REQUIRED COMPONENTS 
    unit_test_framework filesystem system program_options 
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${ASL_SRC_LIBS} )
include_directories( ${ASL_PLAT_REL} )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-std=c++11 ${Boost_DEFINITIONS} -DADOBE_STD_SERIALIZATION -DBOOST_TEST_DYN_LINK" )

# Because Carbon/64 has some different Navigation API calls.
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   message( STATUS "OS X: forcing 32 bit build" )
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")


# These are all standalone, speedy tests with a main.cpp.
# -------------------------------------------------------

# Algorithms
set(ATESTS 
    clamp median select
)
foreach(TEST ${ATESTS})
    set( UNIT_TEST unit-test-${TEST} )
    add_executable( ${UNIT_TEST} algorithm/${TEST}/${TEST}_test.cpp) 
    target_link_libraries( ${UNIT_TEST} asl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST})
endforeach()

set(TESTS 
    any_regular conversion copy_on_write enum_ops equal_range erase forest lower_bound name string
)
foreach(TEST ${TESTS})
    set( UNIT_TEST unit-test-${TEST} )
    add_executable( ${UNIT_TEST} ${TEST}/${TEST}_test.cpp) 
    target_link_libraries( ${UNIT_TEST} asl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST})
endforeach()
