#pragma once

#include <array>
#include <stack>
#include <cstdint>
#include <mutex>

//Native screen dimensions
constexpr unsigned int nWidth = 64;
constexpr unsigned int nHeight = 32;

struct Instruction {
    uint16_t whole;

    Instruction(uint8_t a, uint8_t b) {
        whole = static_cast<uint16_t>(a) << 8 | b;
    }

    template<int N>
    constexpr uint8_t getNibble() {
        return (whole >> (4*N)) & 0xF;
    }

    constexpr uint8_t getFirstNibble() {
        return getNibble<3>();
    }
    constexpr uint8_t getSecondNibble() {
        return getNibble<2>();
    }
    constexpr uint8_t getThirdNibble() {
        return getNibble<1>();
    }
    constexpr uint8_t getFourthNibble() {
        return getNibble<0>();
    }

    constexpr uint16_t getLastThreeNibbles() {
        return whole & 0xFFF;
    }

    template<int N>
    constexpr uint8_t getByte() {
        return (whole >> (8*N)) & 0xFF;
    }

    constexpr uint8_t getFirstByte() {
        return getByte<1>();
    }
    constexpr uint8_t getSecondByte() {
        return getByte<0>();
    }

};

class Chip8 {
    public:
    Chip8();
    void fetchDecodeExecute();
    void mainLoop();
    
    std::array<uint8_t, 4096> emulated_memory;
    uint16_t pc = 512; //12 bits

    uint16_t I_reg; //I registers
    uint8_t VX_reg[16]; //VX registers

    bool key[16]; // pressed keys

    std::stack<uint16_t> stack;

    uint8_t delay_timer;
    uint8_t sound_timer;

    std::array<bool, nWidth*nHeight> framebuffer;
    bool frame_dirty;
    std::mutex frame_mutex;

    bool running = true;
};

extern Chip8 global_chip;
