cmake_minimum_required (VERSION 2.6)

# To see what failed in a test, set the environment variable
# CTEST_OUTPUT_ON_FAILURE
project (asl-test)

find_package( Boost REQUIRED COMPONENTS 
    unit_test_framework filesystem system program_options 
)

message( "ASL_ROOT = ${ASL_ROOT}")
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
set(TESTS 
    any_iterator closed_hash cmath forest_smoke 
    move poly reduction serialization sha stable_partition_selection 
    string_test typeinfo unicode vector xml_parser zuidgen
)
foreach(TEST ${TESTS})
    add_executable( ${TEST} ${TEST}/main.cpp) 
    target_link_libraries( ${TEST} asl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach()

# Tests that require some kind of input files.
# --------------------------------------------

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

# Eve Smoke Test
add_executable (eve_smoke eve_smoke/main.cpp) 
target_link_libraries( eve_smoke asl ${Boost_LIBRARIES} )
file(COPY eve_smoke/default.eve DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files/eve_smoke" )
ADD_TEST(
    NAME eve_smoke 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_files/eve_smoke
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/eve_smoke
)

# Property Model
add_executable (
    property_model_eval 
    property_model_eval/pmeval.cpp 
    property_model_eval/adam_test_parser.cpp 
    property_model_eval/queryable_sheet.cpp 
    property_model_eval/iomanip_flat.cpp)

target_link_libraries( property_model_eval asl ${Boost_LIBRARIES} )
file(COPY property_model_eval 
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files" 
    FILES_MATCHING PATTERN "*pme"
)
ADD_TEST(
    NAME property_model_eval 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_files/property_model_eval
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/property_model_eval *pme
)

# Sudoku!  I'm really impressed by this one.  It's not an automated test though.
add_executable (sudoku sudoku/main.cpp) 
target_link_libraries( sudoku asl ${Boost_LIBRARIES} )
file(COPY sudoku/sudokus.txt DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files/sudoku" )
# ADD_TEST(
#    NAME sudoku 
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_files/sudoku
#    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sudoku
#)

# Tests that require platform.
# ----------------------------
add_executable (expression_filter expression_filter/main.cpp) 
target_link_libraries( expression_filter asl apl ${Boost_LIBRARIES} )
ADD_TEST(expression_filter ${CMAKE_CURRENT_BINARY_DIR}/expression_filter)


#set(SLOW_TESTS dictionary_benchmark n_queens)
#foreach(TEST ${SLOW_TESTS})
#    add_executable( ${TEST} ${TEST}/main.cpp) 
#    target_link_libraries( ${TEST} asl ${Boost_LIBRARIES} )
#    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
#endforeach()
