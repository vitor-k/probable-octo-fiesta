# Definitions for all external bundled libraries

# Suppress warnings from external libraries
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W0)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

# Glad
add_subdirectory(glad)

