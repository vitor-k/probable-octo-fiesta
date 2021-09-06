add_library(getopt SHARED
    getopt.c
    getopt.h
)

create_target_directory_groups(getopt)

target_compile_definitions(getopt PUBLIC IMPORTS_GETOPT)
target_include_directories(getopt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
