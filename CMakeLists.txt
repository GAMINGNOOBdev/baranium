cmake_minimum_required(VERSION 3.0)

project(BaraniumProgrammingLanguage)

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
if (UNIX)
    add_compile_options(
    "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
    "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
    )
endif()
endif()

add_subdirectory(Compiler)
add_subdirectory(Runtime)
add_subdirectory(RuntimeExecutable)