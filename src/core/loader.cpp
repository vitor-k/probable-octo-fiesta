#include <fstream>
#include <fmt/core.h>

#include "loader.h"

void loadChip8Program(Chip8& chip, std::string filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        // get length of file:
        file.seekg (0, file.end);
        int length = file.tellg();
        file.seekg (0, file.beg);

        if (length < 4096 - 512) {
            fmt::print("Reading {} bytes...\n", length);
            file.read((char*) &chip.emulated_memory[512], length);

            if (file) {
                fmt::print("all characters read successfully.\n");
            }
            else {
                fmt::print("error: only {} could be read.\n", file.gcount());
            }
        }
        else {
            fmt::print("File too big.\n");
        }

        file.close();
    }
}
