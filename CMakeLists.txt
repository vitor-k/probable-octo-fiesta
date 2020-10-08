cmake_minimum_required (VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules")

project(POF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set up output paths for executable binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/$<CONFIG>)

if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W3 -O2 -Ot -Oi")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pof)
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()


# Include source code
# ===================

# This function should be passed a list of all files in a target. It will automatically generate
# file groups following the directory hierarchy, so that the layout of the files in IDEs matches the
# one in the filesystem.
function(create_target_directory_groups target_name)
    # Place any files that aren't in the source list in a separate group so that they don't get in
    # the way.
    source_group("Other Files" REGULAR_EXPRESSION ".")

    get_target_property(target_sources "${target_name}" SOURCES)

    foreach(file_name IN LISTS target_sources)
        get_filename_component(dir_name "${file_name}" PATH)
        # Group names use '\' as a separator even though the entire rest of CMake uses '/'...
        string(REPLACE "/" "\\" group_name "${dir_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

add_subdirectory(externals)
add_subdirectory(src)

option(DOWNLOAD_SDL2 "Download SDL2 binaries" ON)

if (DOWNLOAD_SDL2)
    set(SDL2_VER "2.0.12")
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/externals/SDL2-${SDL2_VER}/")
        message(STATUS "Downloading binaries for SDL2...")
        file(DOWNLOAD
            https://www.libsdl.org/release/SDL2-devel-${SDL2_VER}-VC.zip
            "${CMAKE_BINARY_DIR}/externals/SDL2-${SDL2_VER}.zip" SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_BINARY_DIR}/externals/SDL2-${SDL2_VER}.zip"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/externals")
    endif()
    set(SDL2_PREFIX "${CMAKE_BINARY_DIR}/externals/SDL2-${SDL2_VER}")

    set(SDL2_FOUND YES)
    set(SDL2_INCLUDE_DIR "${SDL2_PREFIX}/include" CACHE PATH "Path to SDL2 headers")
    set(SDL2_LIBRARY "${SDL2_PREFIX}/lib/x64/SDL2.lib" CACHE PATH "Path to SDL2 library")
    set(SDL2_DLL_DIR "${SDL2_PREFIX}/lib/x64/" CACHE PATH "Path to SDL2.dll")
else()
    find_package(SDL2 REQUIRED)
endif()
if (SDL2_FOUND)
    add_library(SDL2 INTERFACE)
    target_link_libraries(SDL2 INTERFACE "${SDL2_LIBRARY}")
    target_include_directories(SDL2 INTERFACE "${SDL2_INCLUDE_DIR}")
endif()
