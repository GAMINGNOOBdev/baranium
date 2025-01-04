cmake_minimum_required(VERSION 3.0)

project(baranium)


include_directories(include)

set(baraniumSources
    src/baranium/logging.c
    src/baranium/callback.c
    src/baranium/runtime.c
    src/baranium/script.c
    src/baranium/bcpu.c
    src/baranium/cpu/bcpu_opcodes.c
    src/baranium/cpu/bstack.c
    src/baranium/cpu/bbus.c
    src/baranium/backend/bvarmgr.c
    src/baranium/backend/bfuncmgr.c
    src/baranium/backend/varmath.c
    src/baranium/function.c
    src/baranium/variable.c
    src/baranium/field.c
    )

add_library(baranium STATIC ${baraniumSources})
add_library(baranium-dll SHARED ${baraniumSources})
target_compile_definitions(baranium-dll PUBLIC BARANIUM_DYNAMIC)

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'baranium' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT baranium )
    endif()
endif()