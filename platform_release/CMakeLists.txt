cmake_minimum_required (VERSION 2.6)

project (apl)

# set(Boost_DEBUG 1)
# set(Boost_DETAILED_FAILURE_MSG 1) 
find_package( Boost REQUIRED COMPONENTS filesystem signals system) # thread
message( STATUS "Boost include: ${Boost_INCLUDE_DIR}" )
message( STATUS "Boost defines: ${Boost_DEFINITIONS}" )
message( STATUS "Boost libs:    ${Boost_LIBRARIES}" )

# Used to include FindTBB.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package( TBB )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/adobe )
include_directories( ${ASL_SRC_LIBS} )
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${TBB_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-std=c++11 -Wno-deprecated-declarations ${Boost_DEFINITIONS} -DADOBE_STD_SERIALIZATION" ) 
set(CMAKE_CXX_FLAGS_DEBUG "-DTBB_USE_DEBUG -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

set(CMAKE_C_FLAGS "-Wno-deprecated-declarations" ) 

if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   # Platform specific headers
   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/macintosh )

   # Because Carbon/64 has some different Navigation API calls.
   message( STATUS "OS X: forcing 32 bit build" )
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -isysroot /Developer/SDKs/MacOSX10.6.sdk")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wno-deprecated-declarations")
   set(CMAKE_SHARED_LINKER_FLAGS "-framework AppKit -framework Carbon -framework ApplicationServices -framework AudioToolbox")
endif()

set(PORTABLE_SRCS
        source/dictionary_set.cpp
        source/expression_formatter.cpp
        source/formatter_tokens.cpp
        source/keyboard.cpp
        source/layout_formatter.cpp
        source/property_model_formatter.cpp
        source/sequence_model.cpp

        source/bffd/analyzer.cpp
        source/bffd/common.cpp
        source/bffd/interface.cpp
        source/bffd/parser.cpp
)


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

set(MAC_CPP_SRCS
        macintosh/adobe/future/source/macintosh_controls.cpp
        macintosh/adobe/future/source/macintosh_error.cpp
        macintosh/adobe/future/source/macintosh_events.cpp
        macintosh/adobe/future/source/macintosh_filesystem.cpp
        macintosh/adobe/future/source/macintosh_graphic_utils.cpp
        macintosh/adobe/future/source/platform_drag_and_drop_data.cpp
        macintosh/adobe/future/source/platform_file_monitor_impl.cpp
        macintosh/adobe/future/source/platform_locale_data.cpp
        macintosh/adobe/future/source/platform_number_formatter_data.cpp
        macintosh/adobe/future/source/platform_periodical_data.cpp
        macintosh/adobe/future/widgets/sources/platform_button.cpp
        macintosh/adobe/future/widgets/sources/platform_checkbox.cpp
        macintosh/adobe/future/widgets/sources/platform_display.cpp
        macintosh/adobe/future/widgets/sources/platform_display_number.cpp
        macintosh/adobe/future/widgets/sources/platform_edit_number.cpp
        macintosh/adobe/future/widgets/sources/platform_edit_text.cpp
        macintosh/adobe/future/widgets/sources/platform_group.cpp
        macintosh/adobe/future/widgets/sources/platform_image.cpp
        macintosh/adobe/future/widgets/sources/platform_label.cpp
        macintosh/adobe/future/widgets/sources/platform_link.cpp
        macintosh/adobe/future/widgets/sources/platform_metrics.cpp
        macintosh/adobe/future/widgets/sources/platform_optional_panel.cpp
        macintosh/adobe/future/widgets/sources/platform_panel.cpp
        macintosh/adobe/future/widgets/sources/platform_popup.cpp
        macintosh/adobe/future/widgets/sources/platform_presets.cpp
        macintosh/adobe/future/widgets/sources/platform_preview.cpp
        macintosh/adobe/future/widgets/sources/platform_progress_bar.cpp
        macintosh/adobe/future/widgets/sources/platform_radio_button.cpp
        macintosh/adobe/future/widgets/sources/platform_reveal.cpp
        macintosh/adobe/future/widgets/sources/platform_separator.cpp
        macintosh/adobe/future/widgets/sources/platform_slider.cpp
        macintosh/adobe/future/widgets/sources/platform_tab_group.cpp
        macintosh/adobe/future/widgets/sources/platform_toggle.cpp
        macintosh/adobe/future/widgets/sources/platform_widget_utils.cpp
        macintosh/adobe/future/widgets/sources/platform_window.cpp        
)

set_source_files_properties(
    ${MAC_OBJC_SRCS} 
    PROPERTIES COMPILE_FLAGS
    "-x objective-c++ -Wno-deprecated-declarations")

set_source_files_properties(
    ${MAC_OBJC_SRCS} 
    PROPERTIES LANGUAGE C)

set ( ALL_SRC ${PORTABLE_SRCS} ${WIDGET_SRCS} ${FUTURE_SRCS} )

if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   list(APPEND ALL_SRC ${MAC_OBJC_SRCS})
   list(APPEND ALL_SRC ${MAC_CPP_SRCS})
endif()

add_library(apl SHARED ${ALL_SRC} )
target_link_libraries( apl asl ${TBB_LIBRARY} ${Boost_LIBRARIES} )

