cmake_minimum_required (VERSION 2.6)

message( STATUS "Building ASL - ${CMAKE_BUILD_TYPE}")

project (asl)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( /usr/local/include )

link_directories( ${Boost_LIBRARY_DEFINITIONS} )
link_directories( /usr/lib/system )

set(CMAKE_CXX_FLAGS "-std=c++11 -m64  -Wall -Woverloaded-virtual -Wextra -isystem /opt/local/include -isystem /usr/local/include" )
set(CMAKE_CXX_FLAGS_DEBUG "-DTBB_USE_DEBUG -g -ggdb3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

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

add_library(asl STATIC ${SRCS} )
target_link_libraries( asl tbb )
