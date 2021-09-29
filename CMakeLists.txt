cmake_minimum_required (VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules")

project(POF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set up output paths for executable binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/$<CONFIG>)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pof)
endif()

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
