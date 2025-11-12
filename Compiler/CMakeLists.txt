cmake_minimum_required(VERSION 3.10)

project(barc)

include_directories(../Runtime/include include)

set(barcSources
    src/argument_parser.c
    src/main.c
    )

add_executable(barc ${barcSources})
target_link_libraries(barc baranium-s)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'barc' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT barc )
    endif()
endif()
