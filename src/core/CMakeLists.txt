add_library(core
    chip8.cpp
    chip8.h
    loader.cpp
    loader.h
)

target_link_libraries(core fmt)
