add_library(core SHARED
    chip8.cpp
    chip8.h
    loader.cpp
    loader.h
)

target_compile_definitions(core PUBLIC SHARED_CORE_BUILD)

target_link_libraries(core PRIVATE fmt)
