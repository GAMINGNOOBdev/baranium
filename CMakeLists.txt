cmake_minimum_required(VERSION 3.10)

project(BaraniumProgrammingLanguage)

string(TIMESTAMP NOW "%Y %m %d")
string(REPLACE " " ";" DATE_LIST ${NOW})
list(GET DATE_LIST 0 YEAR)
list(GET DATE_LIST 1 MONTH)
list(GET DATE_LIST 2 DAY)

math(EXPR YEAR ${YEAR})
math(EXPR MONTH ${MONTH})
math(EXPR DAY ${DAY})

add_compile_definitions(__YEAR__=${YEAR})
add_compile_definitions(__MONTH__=${MONTH})
add_compile_definitions(__DAY__=${DAY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

if (UNIX)
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

    add_compile_options(
        "-Wall" "-Wextra" "-Wpedantic" "-fexceptions" "-Wl,--export-dynamic"
    )
endif()

add_subdirectory(Compiler)
add_subdirectory(Runtime)
add_subdirectory(RuntimeExecutable)
add_subdirectory(ProjectManager)
