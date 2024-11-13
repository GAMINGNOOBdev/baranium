cmake_minimum_required(VERSION 3.0)

project(bar)

include_directories(../BGE-File ../Runtime/include include)

set(barSources
    src/argument_parser.c
    src/main.c
    )

add_executable(bar ${barSources})
target_link_libraries(bar baranium)

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'bar' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT bar )
    endif()
endif()