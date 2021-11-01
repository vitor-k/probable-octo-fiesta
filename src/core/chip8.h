#pragma once

#include <array>
#include <chrono>
#include <stack>
#include <cstdint>
#include <mutex>
#include <random>

//Native screen dimensions
constexpr unsigned int nWidth = 64;
constexpr unsigned int nHeight = 32;

class Chip8 {
    public:
    Chip8();
    void mainLoop();
    
    std::array<uint8_t, 4096> emulated_memory;

    void setKey(uint8_t n, bool state);

    void setCoreFrequency(int f);

    bool frameAt(uint8_t x, uint8_t y) const;

    bool frame_dirty; //indicates the framebuffer was modified
    bool isFrameDirty() const;
    void clearDirty();

    std::mutex frame_mutex;

    bool isRunning() const;
    void shutDown();

    void tickDelayTimer();
    void tickSoundTimer();
    void fetchDecodeExecute();

    private:
    void beep();

    uint16_t pc = 512; //12 bits

    uint16_t I_reg; //I register
    uint8_t VX_reg[16]; //VX registers

    std::stack<uint16_t> stack;

    uint8_t delay_timer;
    uint8_t sound_timer;

    bool key[16]; // pressed keys

    std::array<bool, nWidth*nHeight> framebuffer;

    uint32_t micro_wait = 1428; // default 700Hz, 1.428ms

    bool is_running = true;

};

extern Chip8 global_chip;
