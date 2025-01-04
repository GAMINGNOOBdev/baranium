cmake_minimum_required(VERSION 3.0)

project(BaraniumProgrammingLanguage)

if (UNIX)
    add_compile_options(
        "-Wall" "-Wextra" "-Wpedantic" "-Wno-discarded-qualifiers" "-fexceptions"
    )

    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

if (APPLE)
    add_compile_options(
        "-std=c++0x"
    )
endif()

add_subdirectory(Compiler)
add_subdirectory(Runtime)
add_subdirectory(RuntimeExecutable)