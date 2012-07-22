cmake_minimum_required (VERSION 2.6)

project (modal)

find_package( Boost REQUIRED COMPONENTS filesystem signals system )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${ASL_PLAT_REL} )
include_directories( ${ASL_PLAT_REL}/adobe )
include_directories( ${ASL_PLAT_REL}/macintosh )
include_directories( ${ASL_SRC_LIBS} )
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
# include_directories( SYSTEM ${TBB_INCLUDE_DIRS} )

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Boost_DEFINITIONS}" )

#if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   # Platform specific headers
#   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/macintosh )
#endif()

# These are all standalone, speedy tests with a main.cpp.
# -------------------------------------------------------
set(SRCS
    main.cpp
)

#--------------------------------------------------------------------------------
# For Apple set the icns file containing icons
if(APPLE)
    set( OSX_RESOURCES Resources/app_icon.icns ) 
    set_source_files_properties( ${OSX_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties( modal_dialog_interface.nib PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set( SRCS ${SRCS} ${OSX_RESOURCES} )
ENDIF()

# Because Carbon/64 has some different Navigation API calls.
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
   message( STATUS "OS X: forcing 32 bit build" )
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wno-deprecated-declarations -isysroot /Developer/SDKs/MacOSX10.6.sdk")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wno-deprecated-declarations")
   set(CMAKE_EXE_LINKER_FLAGS 
        "-framework AppKit -framework Carbon -framework ApplicationServices -framework AudioToolbox")
endif()

add_executable( modal MACOSX_BUNDLE ${SRCS} )
target_link_libraries( modal apl asl ${Boost_LIBRARIES} )

#if(APPLE)
#    set_target_properties( 
#        modal_dialog_interface 
#        PROPERTIES MACOSX_BUNDLE_INFO_PLIST
#        ${PROJECT_SOURCE_DIR}/Resources/Info.plist )
#ENDIF()
