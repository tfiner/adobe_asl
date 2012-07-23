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
include_directories( SYSTEM ${TBB_INCLUDE_DIRS} )

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
    set_property(TARGET ${UNIT_TEST} PROPERTY FOLDER "asl_unit_tests")
endforeach()

set(TESTS 
    any_regular conversion copy_on_write enum_ops equal_range erase forest lower_bound name string
)
foreach(TEST ${TESTS})
    set( UNIT_TEST unit-test-${TEST} )
    add_executable( ${UNIT_TEST} ${TEST}/${TEST}_test.cpp) 
    target_link_libraries( ${UNIT_TEST} asl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST})
    set_property(TARGET ${UNIT_TEST} PROPERTY FOLDER "asl_unit_tests")
endforeach()
