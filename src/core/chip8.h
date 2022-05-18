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

    void setKey(uint8_t n, bool state);

    void setCoreFrequency(int f);

    bool frameAt(uint8_t x, uint8_t y) const;

    bool isFrameDirty() const;
    void clearDirty();

    bool isRunning() const;
    void shutDown();

    void tickDelayTimer();
    void tickSoundTimer();
    void tickCPU(uint32_t cycles);
    void fetchDecodeExecute();

    private:
    void beep();

    friend void loadChip8Program(Chip8& chip, std::string filename);

    // variables from here
    public:
    std::mutex frame_mutex;
    bool frame_dirty = true; //indicates the framebuffer was modified

    private:
    uint16_t pc = 512; //12 bits

    uint16_t I_reg = 0; //I register
    uint8_t VX_reg[16] = {0}; //VX registers

    std::array<uint8_t, 4096> emulated_memory = { 0 };

    std::stack<uint16_t> stack;

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    bool key[16] = {false}; // pressed keys

    std::array<bool, nWidth*nHeight> framebuffer = {false};

    bool is_running = true;

    uint32_t micro_wait = 1428; // default 700Hz, 1.428ms
};

extern Chip8 global_chip;
