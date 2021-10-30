#pragma once

#include <array>
#include <stack>
#include <cstdint>
#include <mutex>

//Native screen dimensions
constexpr unsigned int nWidth = 64;
constexpr unsigned int nHeight = 32;

class Chip8 {
    public:
    Chip8();
    void mainLoop();
    
    std::array<uint8_t, 4096> emulated_memory;

    bool key[16]; // pressed keys

    uint32_t micro_wait = 1428; // default 700Hz, 1.428ms

    std::array<bool, nWidth*nHeight> framebuffer;
    bool frame_dirty; //indicates the framebuffer was modified
    std::mutex frame_mutex;

    bool is_running = true;

    private:
    void fetchDecodeExecute();
    void beep();

    uint16_t pc = 512; //12 bits

    uint16_t I_reg; //I register
    uint8_t VX_reg[16]; //VX registers

    std::stack<uint16_t> stack;

    uint8_t delay_timer;
    uint8_t sound_timer;
};

extern Chip8 global_chip;
