# Definitions for all external bundled libraries

# Suppress warnings from external libraries
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W0)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

# fmt
add_subdirectory(fmt)

# Glad
add_subdirectory(glad)

# getopt
if (MSVC)
    add_subdirectory(getopt)
endif()
