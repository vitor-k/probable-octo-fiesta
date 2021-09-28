
#ifdef _WIN32
// windows.h needs to be included before shellapi.h
#include <windows.h>

#include <shellapi.h>
#endif

#undef _UNICODE
#include <getopt.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <thread>
#include <memory>

#include "sdl_impl.h"
#include "chip8.h"

#ifdef _WIN32
std::string UTF16ToUTF8(const std::wstring& input) {
    if (input.empty())
        return std::string();

    // first call with cbMultiByte as 0 to get the required size
    const auto size = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                                          nullptr, 0, nullptr, nullptr);

    std::string output(size, '\0');

    // second call writes the string to output
    WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), &output[0],
                        static_cast<int>(output.size()), nullptr, nullptr);

    return output;
}
#endif

static void printHelp(const char* argv0) {
    std::cout << "Usage: " << argv0
              << " [options] <filename>\n"
                 "-h, --help            Display this help text and exit\n";
}

int main(int argc, char* args[]) {
    int option_index = 0;
    char* endarg = nullptr;

#ifdef _WIN32
    int argc_w;
    auto argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);

    if (argv_w == nullptr) {
        std::cout << "Failed to get command line arguments" << std::endl;
        return -1;
    }
#endif

    std::unique_ptr<SDL_impl> impl{std::make_unique<SDL_impl>()};
    std::string filename;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    while (optind < argc) {
        int arg = getopt_long(argc, args, "h", long_options, &option_index);
        if (arg != -1) {
            switch (static_cast<char>(arg)) {
            case 'h':
                printHelp(args[0]);
                return 0;
            }
        } else {
#ifdef _WIN32
            filename = UTF16ToUTF8(argv_w[optind]);
#else
            filepath = argv[optind];
#endif
            std::string extension = filename.substr(filename.rfind('.'));

            optind++;
        }
    }

    if (filename.empty()) {
        std::cout << "Filename not provided. Printing help." << std::endl;
        printHelp(args[0]);
        return 0;
    }

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        // get length of file:
        file.seekg (0, file.end);
        int length = file.tellg();
        file.seekg (0, file.beg);

        if (length < 4096 - 512) {
            std::cout << "Reading " << length << " bytes... " << std::endl;
            file.read((char*) &global_chip.emulated_memory[512], length);

            if (file) {
                std::cout << "all characters read successfully." << std::endl;
            }
            else {
                std::cout << "error: only " << file.gcount() << " could be read" << std::endl;
            }
        }
        else {
            std::cout << "File too big" << std::endl;
            file.close();
            return 0;
        }

        file.close();
    }

    std::thread presentThready([&impl]{impl->Present();});
    std::thread mainThready(&Chip8::mainLoop, &global_chip);
    while(impl->IsOpen()){
        impl->PollEvents();
    }
    global_chip.running = false;
    mainThready.join();
    presentThready.join();

    return 0;
}
