
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
#include <stack>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include "sdl_impl.h"

//Native screen dimensions
constexpr unsigned int nWidth = 64;
constexpr unsigned int nHeight = 32;

uint8_t emulated_memory[4096];
uint16_t pc; //12 bits

uint16_t I_reg; //I registers
uint8_t VX_reg[16]; //VX registers

std::stack<uint16_t> stack;

struct Nibbles {
    int first_nibble : 4;
    int second_nibble : 4;
    int third_nibble : 4;
    int fourth_nibble : 4;
};

struct Bytes {
    uint8_t first_byte;
    uint8_t second_byte;
};

struct NNN {
    int first_nibble : 4;
    int last_three_nibbles : 12;
};

union Instruction {
    uint16_t whole;
    Nibbles nibs;
    Bytes bytes;
    NNN nnnib;
};

void fetchDecodeExecute(){
    //fetch
    Instruction insty = *(Instruction*) (emulated_memory+pc);
    pc += 2;

    //decode
    switch(insty.nibs.first_nibble){
        case 0x0:
            if(insty.whole == 0x00E0) {
                // clear screen
            }
            else if(insty.whole == 0x00EE) {
                // return from subroutine
                pc = stack.top();
                stack.pop();
            }
            else {
                // 0NNN execute subroutine
                stack.push(pc);
                pc = insty.nnnib.last_three_nibbles;
            }
            break;
        case 0x1: // 1NNN jump
            pc = insty.nnnib.last_three_nibbles;
            break;
        case 0x6: // 6XNN set register VX
            VX_reg[insty.nibs.second_nibble] = insty.bytes.second_byte;
            break;
        case 0x7: // 7XNN add value to register VX
            VX_reg[insty.nibs.second_nibble] += insty.bytes.second_byte;
            break;
        case 0xA: // ANNN set index register I
            I_reg = insty.nnnib.last_three_nibbles;
            break;
        case 0xD: // DXYN display/draw
            break;
        default:
            printf("Unhandled opcode %d", insty);
    }
}

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

    SDL_impl impl;
    std::string filename;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
    };

    while (optind < argc) {
        int arg = getopt_long(argc, args, "h", long_options, &option_index);
        int tmp;
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
            file.read((char*) &emulated_memory[512], length);

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

    return 0;
}
