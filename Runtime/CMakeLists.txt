cmake_minimum_required(VERSION 3.0)

project(bgs)

include_directories(../include include)

set(bgsSources
    src/baranium/runtime.c
    src/baranium/script.c
    src/baranium/bcpu.c
    src/test.c
    )

add_executable(bgs ${bgsSources})

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'bgs' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT bgs )
    endif()
endif()