cmake_minimum_required (VERSION 2.6)

project (asl)

# set(Boost_DEBUG 1)
# set(Boost_DETAILED_FAILURE_MSG 1) 
find_package( Boost REQUIRED COMPONENTS thread signals )

# Used to include FindTBB.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package( TBB )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${TBB_INCLUDE_DIRS} )

set(SRCS 
        source/adam.cpp
        source/adam_evaluate.cpp
        source/adam_parser.cpp
        source/any_regular.cpp
        source/empty.cpp
        source/eve.cpp
        source/eve_evaluate.cpp
        source/eve_parser.cpp
        source/expression_filter.cpp
        source/expression_parser.cpp
        source/extents.cpp
        source/external_model.cpp
        source/iomanip.cpp
        source/iomanip_asl_cel.cpp
        source/iomanip_javascript.cpp
        source/iomanip_pdf.cpp
        source/iomanip_xml.cpp
        source/istream.cpp
        source/lex_shared.cpp
        source/lex_stream.cpp
        source/localization.cpp
        source/md5.cpp
        source/memory.cpp
        source/name.cpp
        source/parser_shared.cpp
        source/string.cpp
        source/string_pool.cpp
        source/test_configuration.cpp
        source/thread_id.cpp
        source/token.cpp
        source/typeinfo.cpp
        source/unicode.cpp
        source/virtual_machine.cpp
        source/xml_lex.cpp
        source/xml_parser.cpp
        source/xml_token.cpp
        source/xstring.cpp
        source/zuid.cpp
        source/zuid_sys_dep.cpp
        source/zuid_uuid.cpp
)


# add_library(asl SHARED ${SRCS} )
add_library(asl STATIC ${SRCS} )
set_property(TARGET asl PROPERTY FOLDER "libs")

target_link_libraries( asl ${TBB_LIBRARY_DEBUG} ${Boost_LIBRARIES} )

