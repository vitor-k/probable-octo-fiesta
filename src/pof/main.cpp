
#ifdef _WIN32
#include <windows.h>
#endif

#undef _UNICODE
#include <getopt.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <fstream>
#include <cstdint>
#include <string>
#include <thread>

#include <fmt/core.h>

#include "sdl_impl.h"
#include "core/chip8.h"
#include "core/loader.h"

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
    fmt::print("Usage: {} [options] <filename>\n"
               "-h, --help            Display this help text and exit\n",
               argv0);
}

int main(int argc, char* args[]) {
    int option_index = 0;
    char* endarg = nullptr;

#ifdef _WIN32
    int argc_w;
    auto argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);

    if (argv_w == nullptr) {
        fmt::print("Failed to get command line arguments\n");
        return -1;
    }
#endif

    std::unique_ptr<SDL_impl> impl{std::make_unique<SDL_impl>()};
    std::string filename;

    static struct option long_options[] = {
        {"slow", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    while (optind < argc) {
        int arg = getopt_long(argc, args, "h", long_options, &option_index);
        if (arg != -1) {
            switch (static_cast<char>(arg)) {
            case 's':
                global_chip.setCoreFrequency(350); // 350Hz, 2.857ms
            case 'h':
                printHelp(args[0]);
                return 0;
            }
        } else {
#ifdef _WIN32
            filename = UTF16ToUTF8(argv_w[optind]);
#else
            filename = args[optind];
#endif
            std::string extension = filename.substr(filename.rfind('.'));

            optind++;
        }
    }

    if (filename.empty()) {
        fmt::print("Filename not provided. Printing help.\n");
        printHelp(args[0]);
        return 0;
    }

    loadChip8Program(global_chip, filename);

    std::thread presentThready([&impl]{impl->Present();});
    std::thread mainThready(&Chip8::mainLoop, &global_chip);
    while(impl->IsOpen()){
        impl->PollEvents();
    }
    global_chip.shutDown();
    mainThready.join();
    presentThready.join();

    return 0;
}
