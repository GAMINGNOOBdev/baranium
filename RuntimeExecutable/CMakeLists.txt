cmake_minimum_required(VERSION 3.10)

project(bar)

include_directories(../Runtime/include include)

set(barSources
    src/argument_parser.c
    src/callbacks.c
    src/main.c
    )

add_executable(bar ${barSources})
target_link_libraries(bar baranium-s)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if (UNIX)
    target_link_libraries(bar m)
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'bar' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT bar )
    endif()
endif()
