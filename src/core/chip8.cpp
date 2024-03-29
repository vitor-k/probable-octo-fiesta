#include "chip8.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>

#include <fmt/core.h>

#define CHIP8_NEW_SHIFT

Chip8 global_chip;

namespace {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine randy(seed);

    auto timer_previous_time = std::chrono::system_clock::now().time_since_epoch();
    auto main_previous_time = std::chrono::system_clock::now().time_since_epoch();

    constexpr std::array<uint8_t, 80> font = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    constexpr uint16_t font_starting_address = 0x50;

} // Anonymous namespace

struct Instruction {
    uint16_t whole{0};

    constexpr Instruction(uint8_t a, uint8_t b) {
        whole = static_cast<uint16_t>(a) << 8 | b;
    }

    template<int N>
    constexpr uint8_t getNibble() const {
        return (whole >> (4*N)) & 0xF;
    }

    constexpr uint8_t getFirstNibble() const {
        return getNibble<3>();
    }
    constexpr uint8_t getSecondNibble() const {
        return getNibble<2>();
    }
    constexpr uint8_t getThirdNibble() const {
        return getNibble<1>();
    }
    constexpr uint8_t getFourthNibble() const {
        return getNibble<0>();
    }

    constexpr uint16_t getLastThreeNibbles() const {
        return whole & 0xFFF;
    }

    template<int N>
    constexpr uint8_t getByte() const {
        return (whole >> (8*N)) & 0xFF;
    }

    constexpr uint8_t getFirstByte() const {
        return getByte<1>();
    }
    constexpr uint8_t getSecondByte() const {
        return getByte<0>();
    }

};

void printUnhandledOpcode(Instruction opcode) {
    fmt::print("Unhandled opcode {:#06x}\n", opcode.whole);
}

Chip8::Chip8() {
    // Font
    auto font_address = emulated_memory.begin() + font_starting_address;

    std::copy(font.begin(), font.end(), font_address);
}

void Chip8::beep() {
    fmt::print("beep\n");
}

void Chip8::setKey(uint8_t n, bool state) {
    if (n <= 0xF) {
        key[n] = state;
    }
}

void Chip8::setCoreFrequency(int f) {
    micro_wait = 1000000 / f;
}

bool Chip8::frameAt(uint8_t x, uint8_t y) const {
    return framebuffer[y*nWidth + x];
}

bool Chip8::isFrameDirty() const {
    return frame_dirty;
}

void Chip8::clearDirty() {
    frame_dirty = false;
}

bool Chip8::isRunning() const {
    return is_running;
}

void Chip8::shutDown() {
    is_running = false;
}

void Chip8::tickDelayTimer() {
    if (delay_timer > 0) {
        delay_timer--;
    }
}

void Chip8::tickSoundTimer() {
    if (sound_timer > 0) {
        sound_timer--;
        beep();
    }
}

void Chip8::tickCPU(uint32_t cycles) {
    for(uint32_t i = 0; i < cycles; i++){
        fetchDecodeExecute();
    }
}

void Chip8::fetchDecodeExecute() {
    if (pc >= 4096) {
        fmt::print("Out of bounds pc\n");
        is_running = false;
        return;
    }

    //fetch
    const Instruction insty(emulated_memory[pc], emulated_memory[pc+1]);

    pc += 2;

    //decode
    const auto first_nibble = insty.getFirstNibble();
    switch(first_nibble){
        case 0x0:
            if(insty.whole == 0x00E0) {
                // clear screen
                framebuffer.fill(false);
                frame_dirty = true;
            }
            else if(insty.whole == 0x00EE) {
                // return from subroutine
                pc = stack.top();
                stack.pop();
            }
            else {
                // 0NNN execute subroutine
                // do not implement
                printUnhandledOpcode(insty);
            }
            break;
        case 0x1: // 1NNN jump
            pc = insty.getLastThreeNibbles();
            break;
        case 0x2: // 2NNN call subroutine
            stack.push(pc);
            pc = insty.getLastThreeNibbles();
            break;
        case 0x3: // 3XNN skip equal
            if(VX_reg[insty.getSecondNibble()] == insty.getSecondByte()) {
                pc += 2;
            }
            break;
        case 0x4: // 4XNN skip not equal
            if(VX_reg[insty.getSecondNibble()] != insty.getSecondByte()) {
                pc += 2;
            }
            break;
        case 0x5: // 5XY0 skip if Vx == Vy
            if(VX_reg[insty.getSecondNibble()] == VX_reg[insty.getThirdNibble()]) {
                pc += 2;
            }
            break;
        case 0x9: // 9XY0 skip if Vx != Vy
            if(VX_reg[insty.getSecondNibble()] != VX_reg[insty.getThirdNibble()]) {
                pc += 2;
            }
            break;
        case 0x6: // 6XNN set register VX
            VX_reg[insty.getSecondNibble()] = insty.getSecondByte();
            break;
        case 0x7: // 7XNN add value to register VX
            VX_reg[insty.getSecondNibble()] += insty.getSecondByte();
            break;
        case 0x8:
            switch(insty.getFourthNibble()) {
                case 0x0: // 8XY0 set
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getThirdNibble()];
                    break;
                case 0x1: // 8XY1 bitwise logical OR
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getSecondNibble()] | VX_reg[insty.getThirdNibble()];
                    break;
                case 0x2: // 8XY2 bitwise logical AND
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getSecondNibble()] & VX_reg[insty.getThirdNibble()];
                    break;
                case 0x3: // 8XY3 bitwise logical XOR
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getSecondNibble()] ^ VX_reg[insty.getThirdNibble()];
                    break;
                case 0x4: // 8XY4 add
                    {
                        const uint16_t sum = static_cast<uint16_t>(VX_reg[insty.getSecondNibble()]) + VX_reg[insty.getThirdNibble()];
                        VX_reg[0xF] = (sum > 255);
                        VX_reg[insty.getSecondNibble()] = static_cast<uint8_t>(sum % 256);
                    }
                    break;
                case 0x5: // 8XY5 subtract
                    {
                        const uint8_t vx = VX_reg[insty.getSecondNibble()];
                        const uint8_t vy = VX_reg[insty.getThirdNibble()];
                        VX_reg[insty.getSecondNibble()] = vx - vy;
                        VX_reg[0xF] = (vx >= vy);
                    }
                    break;
                case 0x7: // 8XY7 subtract
                    {
                        const uint8_t vx = VX_reg[insty.getSecondNibble()];
                        const uint8_t vy = VX_reg[insty.getThirdNibble()];
                        VX_reg[insty.getSecondNibble()] = vy - vx;
                        VX_reg[0xF] = (vy >= vx);
                    }
                    break;
                case 0x6: //8XY6 right shift
#ifndef CHIP8_NEW_SHIFT
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getThirdNibble()];
#endif
                    VX_reg[0xF] = VX_reg[insty.getSecondNibble()] & 1;
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getSecondNibble()] >> 1;
                    break;
                case 0xE: //8XYE left shift
#ifndef CHIP8_NEW_SHIFT
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getThirdNibble()];
#endif
                    VX_reg[0xF] = (VX_reg[insty.getSecondNibble()] & 0x80) >> 7;
                    VX_reg[insty.getSecondNibble()] = VX_reg[insty.getSecondNibble()] << 1;
                    break;
                default:
                    printUnhandledOpcode(insty);
            }
            break;
        case 0xA: // ANNN set index register I
            I_reg = insty.getLastThreeNibbles();
            break;
        case 0xB: // BNNN jump with offset
#ifdef CHIP8_QUIRKY_JUMP
            pc = insty.getLastThreeNibbles() + VX_reg[insty.getSecondNibble()];
#else
            pc = insty.getLastThreeNibbles() + VX_reg[0];
#endif
            break;
        case 0xC: // CXNN random
            VX_reg[insty.getSecondNibble()] = randy() & insty.getSecondByte();
            break;
        case 0xD: // DXYN display/draw
        {
            const int x = VX_reg[insty.getSecondNibble()] % nWidth;
            const int y = VX_reg[insty.getThirdNibble()] % nHeight;
            const int n = insty.getFourthNibble();
            bool unset = false;

            frame_mutex.lock();
            for(int i=0; i<n; i++) {
                uint8_t sprite = emulated_memory[I_reg+i];
                for(int j=0; j<8; j++) {
                    bool bit = (sprite >> (7-j)) & 1;
                    int i_y = y+i;
                    int j_x = x+j;
                    if (i_y < nHeight && j_x < nWidth) {
                        bool fbit = framebuffer[i_y * nWidth + j_x];
                        if(bit && fbit) {
                            unset = true;
                        }
                        framebuffer[i_y * nWidth + j_x] = bit ^ fbit;
                    }
                }
            }
            frame_mutex.unlock();
            VX_reg[0xF] = unset;

            frame_dirty = true;
        }
            break;
        case 0xE:
            if(insty.getSecondByte() == 0x9E) { // EX9E skip if key pressed
                if(key[VX_reg[insty.getSecondNibble()]]) {
                    pc += 2;
                }
            }
            else if(insty.getSecondByte() == 0xA1) { // EXA1 skip if key not pressed
                if(!key[VX_reg[insty.getSecondNibble()]]) {
                    pc += 2;
                }
            }
            else {
                printUnhandledOpcode(insty);
            }
            break;
        case 0xF:
            switch(insty.getSecondByte()) {
                case 0x07: // FX07 get delay timer
                    VX_reg[insty.getSecondNibble()] = delay_timer;
                    break;
                case 0x15: // FX15 set delay timer
                    delay_timer = VX_reg[insty.getSecondNibble()];
                    break;
                case 0x18: // FX18 set sound timer
                    sound_timer = VX_reg[insty.getSecondNibble()];
                    break;
                case 0x0A: // FX0A get key
                    {
                        bool pressed = false;
                        for (int i=0; i<16; i++) {
                            if(key[i]){
                                pressed = true;
                                VX_reg[insty.getSecondNibble()] = i;
                                break;
                            }
                        }
                        if(!pressed)
                            pc-=2;
                    }
                    break;
                case 0x1E: // FX1E add to index
                    {
                    I_reg += VX_reg[insty.getSecondNibble()];
                    }
                    break;
                case 0x29: // FX29 font character
                    {
                    const uint8_t x = VX_reg[insty.getSecondNibble()];
                    I_reg = font_starting_address + 5*(x & 0xF);
                    }
                    break;
                case 0x33: // FX33 decimal conversion
                    {
                    const uint8_t number = VX_reg[insty.getSecondNibble()];
                    emulated_memory[I_reg] = number / 100;
                    emulated_memory[I_reg+1] = (number % 100) / 10;
                    emulated_memory[I_reg+2] = number % 10;
                    }
                    break;
                case 0x55: // FX55 store in memory
                    {
                    const uint8_t x = insty.getSecondNibble();
                    for(int i=0; i<=x; i++) {
                        emulated_memory[I_reg+i] = VX_reg[i];
                    }
#ifdef CHIP8_LOAD_STORE
                    I_reg += x;
#endif
                    }
                    break;
                case 0x65: // FX65 load from memory
                    {
                    const uint8_t x = insty.getSecondNibble();
                    for(int i=0; i<=x; i++) {
                        VX_reg[i] = emulated_memory[I_reg+i];
                    }
#ifdef CHIP8_LOAD_STORE
                    I_reg += x;
#endif
                    }
                    break;
                default:
                    printUnhandledOpcode(insty);
            }
            break;
        default:
            printUnhandledOpcode(insty);
    }
}

void Chip8::mainLoop() {
    const uint32_t default_sleep = 16666u;
    uint32_t recommended_sleep = default_sleep;
    while(is_running) {
        using namespace std::chrono;
        auto current_time = system_clock::now().time_since_epoch();
        auto time_difference = duration_cast<microseconds>(current_time - timer_previous_time).count();
        if(time_difference > 16666) { // 60Hz, 16.666ms
            tickCPU(16666u/micro_wait);
            tickDelayTimer();
            tickSoundTimer();
            timer_previous_time = current_time;
        }
        else {
            recommended_sleep = std::min(recommended_sleep, static_cast<uint32_t>(time_difference / 2u));
        }

        if(recommended_sleep > default_sleep / 2) {
            std::this_thread::sleep_for(std::chrono::microseconds(default_sleep / 2));
        }
        recommended_sleep = default_sleep;
    }
}
