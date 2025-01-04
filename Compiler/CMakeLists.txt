cmake_minimum_required(VERSION 3.0)

project(barc)

set(barcSources
    src/Language/AbstractSyntaxTree.cpp
    src/Language/AST_TokenParser.cpp
    src/Language/IfElseStatement.cpp
    src/Binaries/NameLookupTable.cpp
    src/Binaries/CompiledScript.cpp
    src/Binaries/VariableTable.cpp
    src/Binaries/CodeBuilder.cpp
    src/Language/Expression.cpp
    src/Binaries/Compiler.cpp
    src/Language/Language.cpp
    src/Language/Variable.cpp
    src/Language/Function.cpp
    src/Language/Token.cpp
    src/Language/Field.cpp
    src/Language/Loop.cpp
    src/ArgumentParser.cpp
    src/MemoryManager.cpp
    src/Preprocessor.cpp
    src/SourceToken.cpp
    src/TokenParser.cpp
    src/StringUtil.cpp
    src/FileUtil.cpp
    src/Logging.cpp
    src/Source.cpp
    src/main.cpp
    )

add_executable(barc ${barcSources})
add_compile_definitions(BGEFILE_SILENCED)

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\nYou are running a cmake version lower than 3.6.0, you have to set 'barc' as the Startup project manually.\n")
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT barc )
    endif()
endif()