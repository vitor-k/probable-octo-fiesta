add_executable(pof
    main.cpp
    sdl_impl.cpp
)

create_target_directory_groups(pof)

target_link_libraries(pof PRIVATE ${PLATFORM_LIBRARIES} SDL2 glad getopt)

if (MSVC)
    include(WindowsCopyFiles)
    windows_copy_files(pof ${SDL2_DLL_DIR} "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/" SDL2.dll)
endif()
