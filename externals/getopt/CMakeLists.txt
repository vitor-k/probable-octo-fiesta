add_library(getopt SHARED
    getopt.c
    getopt.h
)

target_compile_definitions(getopt PUBLIC IMPORTS_GETOPT)
target_include_directories(getopt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
