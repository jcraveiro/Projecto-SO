set (PROJECT_NAME "so-project")

set (CMAKE_C_FLAGS "-Werror -Wall -D_REENTRANT -g -Wstrict-prototypes")

project (${PROJECT_NAME})
cmake_minimum_required (VERSION 2.8)

file (GLOB_RECURSE PROJECT_SOURCES "src/*.c")

include_directories (header)
add_executable (${PROJECT_NAME} ${PROJECT_SOURCES})
