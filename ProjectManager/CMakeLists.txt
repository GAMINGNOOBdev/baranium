cmake_minimum_required(VERSION 3.10)

project(barrel)

include_directories(../Runtime/include include)

set(barrelSources
    src/cmd/defines.c
    src/cmd/install.c
    src/cmd/config.c
    src/cmd/create.c
    src/cmd/build.c
    src/cmd/help.c
    src/cmd/lib.c
    src/cmd/run.c

    src/operations.c
    src/config.c
    src/util.c
    src/main.c
    )

add_executable(barrel ${barrelSources})
target_link_libraries(barrel baranium-s)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if (UNIX)
    target_link_libraries(barrel m)
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'barrel' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT barrel )
    endif()
endif()
