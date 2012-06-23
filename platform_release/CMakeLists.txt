cmake_minimum_required (VERSION 2.6)

message( STATUS "Building Adobe Platform - ${CMAKE_BUILD_TYPE}")

project (apl)

# set(Boost_DEBUG 1)
# set(Boost_DETAILED_FAILURE_MSG 1) 
find_package( Boost REQUIRED COMPONENTS filesystem signals ) # thread
message( STATUS "Boost include: ${Boost_INCLUDE_DIR}" )
message( STATUS "Boost defines: ${Boost_DEFINITIONS}" )
message( STATUS "Boost libs:    ${Boost_LIBRARIES}" )

# Used to include FindTBB.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package( TBB )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/macintosh )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../adobe_source_libraries )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${TBB_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-std=c++11 -m32 -Wno-deprecated-declarations ${Boost_DEFINITIONS}" ) 
# set(CMAKE_CXX_FLAGS "-std=c++11 -m32 -Wall -Woverloaded-virtual -Wextra ${Boost_DEFINITIONS}" ) 
set(CMAKE_CXX_FLAGS_DEBUG "-DTBB_USE_DEBUG -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

set(WIDGET_SRCS
        adobe/future/widgets/sources/alert.cpp
        adobe/future/widgets/sources/button_factory.cpp
        adobe/future/widgets/sources/button_helper.cpp
        adobe/future/widgets/sources/checkbox_factory.cpp
        adobe/future/widgets/sources/control_button.cpp
        adobe/future/widgets/sources/control_button_factory.cpp
        adobe/future/widgets/sources/default_factory.cpp
        adobe/future/widgets/sources/display_number_factory.cpp
        adobe/future/widgets/sources/edit_number.cpp
        adobe/future/widgets/sources/edit_number_factory.cpp
        adobe/future/widgets/sources/edit_text_factory.cpp
        adobe/future/widgets/sources/group_factory.cpp
        adobe/future/widgets/sources/image_factory.cpp
        adobe/future/widgets/sources/label_factory.cpp
        adobe/future/widgets/sources/link_factory.cpp
        adobe/future/widgets/sources/number_unit.cpp
        adobe/future/widgets/sources/optional_panel_factory.cpp
        adobe/future/widgets/sources/panel_factory.cpp
        adobe/future/widgets/sources/popup_factory.cpp
        adobe/future/widgets/sources/presets_common.cpp
        adobe/future/widgets/sources/presets_factory.cpp
        adobe/future/widgets/sources/preview_factory.cpp
        adobe/future/widgets/sources/progress_bar_factory.cpp
        adobe/future/widgets/sources/radio_button_factory.cpp
        adobe/future/widgets/sources/reveal_factory.cpp
        adobe/future/widgets/sources/separator_factory.cpp
        adobe/future/widgets/sources/slider_factory.cpp
        adobe/future/widgets/sources/sublayout.cpp
        adobe/future/widgets/sources/tab_group_factory.cpp
        adobe/future/widgets/sources/toggle_factory.cpp
        adobe/future/widgets/sources/virtual_machine_extension.cpp
        adobe/future/widgets/sources/widget_factory.cpp
        adobe/future/widgets/sources/widget_factory_registry.cpp
        adobe/future/widgets/sources/widget_tokens.cpp
        adobe/future/widgets/sources/widget_utils.cpp
        adobe/future/widgets/sources/window_factory.cpp
        adobe/future/widgets/sources/window_server.cpp 
)

set(FUTURE_SRCS
        adobe/future/source/behavior.cpp
        adobe/future/source/cmd_system.cpp
        adobe/future/source/cursor_stack.cpp
        adobe/future/source/cursor_stack.hpp
        adobe/future/source/drag_and_drop.cpp
        adobe/future/source/image_slurp.cpp
        adobe/future/source/locale.cpp
        adobe/future/source/menu_system.cpp
        adobe/future/source/modal_dialog_interface.cpp
        adobe/future/source/resources.cpp        
)

set(MAC_OBJC_SRCS
        macintosh/adobe/future/source/macintosh_cocoa_bridge.mm
        macintosh/adobe/future/source/platform_cursor.mm
)

SET_SOURCE_FILES_PROPERTIES(
    ${MAC_OBJC_SRCS}
    PROPERTIES LANGUAGE C
)

add_library(apl SHARED ${WIDGET_SRCS} ${FUTURE_SRCS} ${MAC_OBJC_SRCS} )
target_link_libraries( apl ${TBB_LIBRARY} ${Boost_LIBRARIES} )

