add_executable(pof
    main.cpp
    sdl_impl.cpp
    sdl_impl.h
)

target_link_libraries(pof PRIVATE core SDL2 glad fmt)

find_package(Threads REQUIRED)
target_link_libraries(pof PRIVATE Threads::Threads)

if (MSVC)
    target_link_libraries(pof PRIVATE getopt)
    include(WindowsCopyFiles)
    windows_copy_files(pof ${SDL2_DLL_DIR} "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/" SDL2.dll)
endif()
