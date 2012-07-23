cmake_minimum_required (VERSION 2.6)

project (apl-test)

find_package( Boost REQUIRED COMPONENTS filesystem signals system unit_test_framework )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${ASL_PLAT_REL} )
include_directories( ${ASL_PLAT_REL}/adobe )
include_directories( ${ASL_PLAT_REL}/macintosh )
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
#    modal_dialog_interface
    notify_on_write
    property_model_tidy 
    rset
    selection_ops 
    sequence_model 
    sequence_mvc_muldex 
    xml_to_forest 
    xstr_bench
    xstr_test
)

# Running from the test's source dir eliminates the need to copy any files.
foreach(TEST ${TESTS})
    add_executable( ${TEST} ${TEST}/main.cpp) 
    target_link_libraries( ${TEST} apl ${Boost_LIBRARIES} )
    add_test(
        NAME ${TEST} 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TEST}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${TEST} )
    set_property(TARGET ${TEST} PROPERTY FOLDER "apl_test")
endforeach()

# In progress, eh, looks like this one needs too much work to get compiled.
# I think it just needs revamping with boost::concepts.
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

