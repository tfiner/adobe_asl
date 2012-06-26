cmake_minimum_required (VERSION 2.6)

project (apl-test)

find_package( Boost REQUIRED COMPONENTS filesystem signals system unit_test_framework )

# Used to include FindTBB.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package( TBB )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${ASL_PLAT_REL} )
include_directories( ${ASL_PLAT_REL}/adobe )
include_directories( ${ASL_SRC_LIBS} )
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${TBB_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Boost_DEFINITIONS}" )

if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   # Platform specific headers
   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/macintosh )
endif()

# These are all standalone, speedy tests with a main.cpp.
# -------------------------------------------------------
set(TESTS
    function_pack
    notify_on_write 
    rset
    selection_ops 
    sequence_model 
    sequence_mvc_muldex 
    xml_to_forest 
    xstr_bench
)
foreach(TEST ${TESTS})
    add_executable( ${TEST} ${TEST}/main.cpp) 
    target_link_libraries( ${TEST} apl ${Boost_LIBRARIES} )
    add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach()

add_executable (xstr_test xstr_test/main.cpp) 
target_link_libraries( xstr_test apl ${Boost_LIBRARIES} )
file(COPY xstr_test/default.xstr DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_files/xstr_test" )
ADD_TEST(
    NAME xstr_test 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_files/xstr_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/xstr_test
)

# In progress
# add_executable (sentences 
#    sentences/main.cpp
#    sentences/command_system.cpp
#    sentences/expression_sentence.cpp
#) 
#target_link_libraries( sentences apl ${Boost_LIBRARIES} )
#ADD_TEST(
#    NAME sentences 
#    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sentences
#)

