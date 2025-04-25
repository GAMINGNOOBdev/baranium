cmake_minimum_required(VERSION 3.10)

project(baranium)

include_directories(include)

set(baraniumSources
    src/baranium/backend/bvarmgr.c
    src/baranium/backend/bfuncmgr.c
    src/baranium/backend/dynlibloader.c
    src/baranium/backend/varmath.c
    src/baranium/compiler/binaries/compiler.c
    src/baranium/compiler/binaries/symbol_table.c
    src/baranium/compiler/language/abstract_syntax_tree.c
    src/baranium/compiler/language/expression_token.c
    src/baranium/compiler/language/field_token.c
    src/baranium/compiler/language/function_token.c
    src/baranium/compiler/language/if_else_token.c
    src/baranium/compiler/language/language.c
    src/baranium/compiler/language/loop_token.c
    src/baranium/compiler/language/token.c
    src/baranium/compiler/language/variable_token.c
    src/baranium/compiler/compiler_context.c
    src/baranium/compiler/preprocessor.c
    src/baranium/compiler/source_token.c
    src/baranium/compiler/source.c
    src/baranium/compiler/token_parser.c
    src/baranium/cpu/bbus.c
    src/baranium/cpu/bcpu_opcodes.c
    src/baranium/cpu/bstack.c
    src/baranium/bcpu.c
    src/baranium/callback.c
    src/baranium/field.c
    src/baranium/file_util.c
    src/baranium/function.c
    src/baranium/library.c
    src/baranium/logging.c
    src/baranium/runtime.c
    src/baranium/script.c
    src/baranium/string_util.c
    src/baranium/variable.c
    )

add_library(baranium SHARED ${baraniumSources})
add_library(baranium-s STATIC ${baraniumSources})
target_compile_definitions(baranium PUBLIC BARANIUM_DYNAMIC)

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'baranium' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT baranium )
    endif()
endif()
