#include "chip8.h"
#include <cstdio>
#include <chrono>
#include <random>

Chip8 global_chip;

namespace {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine randy(seed);
} // Anonymous namespace

constexpr uint16_t font_starting_address = 0x50;

Chip8::Chip8() {
    // Font
    auto font_address = emulated_memory.begin() + font_starting_address;

    std::array<uint8_t, 80> font = {
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

    std::copy(font.begin(), font.end(), font_address);

}

void Chip8::fetchDecodeExecute() {
    //fetch
    Instruction insty(emulated_memory[pc], emulated_memory[pc+1]);

    pc += 2;

    //decode
    auto first_nibble = insty.getFirstNibble();
    switch(first_nibble){
        case 0x0:
            if(insty.whole == 0x00E0) {
                // clear screen
                for(int i=0; i< nWidth*nHeight; i++) {
                    framebuffer[i] = false;
                }
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
                        uint16_t sum = static_cast<uint16_t>(VX_reg[insty.getSecondNibble()]) + VX_reg[insty.getThirdNibble()];
                        VX_reg[0xF] = (sum > 255);
                        VX_reg[insty.getSecondNibble()] = static_cast<uint8_t>(sum % 256);
                    }
                    break;
                case 0x5: // 8XY5 subtract
                    {
                        uint8_t vx = VX_reg[insty.getSecondNibble()];
                        uint8_t vy = VX_reg[insty.getThirdNibble()];
                        VX_reg[0xF] = (vx >= vy);
                        VX_reg[insty.getSecondNibble()] = vx - vy;
                    }
                    break;
                case 0x7: // 8XY7 subtract
                    {
                        uint8_t vx = VX_reg[insty.getSecondNibble()];
                        uint8_t vy = VX_reg[insty.getThirdNibble()];
                        VX_reg[0xF] = (vy >= vx);
                        VX_reg[insty.getSecondNibble()] = vy - vx;
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
                    printf("Unhandled opcode %#6x\n", insty.whole);
            }
            break;
        case 0xA: // ANNN set index register I
            I_reg = insty.getLastThreeNibbles();
            break;
        case 0xB: // BNNN jump with offset
            pc = insty.getLastThreeNibbles() + VX_reg[0];
            break;
        case 0xC: // CXNN random
            VX_reg[insty.getSecondNibble()] = randy() & insty.getSecondByte();
            break;
        case 0xD: // DXYN display/draw
        {
            int x = VX_reg[insty.getSecondNibble()] % nWidth;
            int y = VX_reg[insty.getThirdNibble()] % nHeight;
            int n = insty.getFourthNibble();
            bool unset = false;

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
            VX_reg[0xF] = unset;

            frame_dirty = true;
        }
            break;
        case 0xF:
            switch(insty.getSecondByte()) {
                case 0x1E: // FX1E add to index
                    {
                    I_reg += VX_reg[insty.getSecondNibble()];
                    }
                    break;
                case 0x29: // FX29 font character
                    {
                    uint8_t x = VX_reg[insty.getSecondNibble()];
                    I_reg = font_starting_address + 5*(x & 0xF);
                    }
                    break;
                case 0x33: // FX33 decimal conversion
                    {
                    uint8_t number = VX_reg[insty.getSecondNibble()];
                    emulated_memory[I_reg] = number / 100;
                    emulated_memory[I_reg+1] = (number % 100) / 10;
                    emulated_memory[I_reg+2] = number % 10;
                    }
                    break;
                case 0x55: // FX55 store in memory
                    {
                    uint8_t x = insty.getSecondNibble();
                    for(int i=0; i<=x; i++) {
                        emulated_memory[I_reg+i] = VX_reg[i];
                    }
                    }
                    break;
                case 0x65: // FX65 load from memory
                    {
                    uint8_t x = insty.getSecondNibble();
                    for(int i=0; i<=x; i++) {
                        VX_reg[i] = emulated_memory[I_reg+i];
                    }
                    }
                    break;
                default:
                    printf("Unhandled opcode %#6x\n", insty.whole);
            }
            break;
        default:
            printf("Unhandled opcode %#6x\n", insty.whole);
    }
}
