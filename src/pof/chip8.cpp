#include "chip8.h"
#include <cstdio>
#include <chrono>
#include <random>

Chip8 global_chip;

namespace {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine randy(seed);
} // Anonymous namespace

Chip8::Chip8() {
    // Font
    auto font_address = emulated_memory.begin() + 0x050;

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
        case 0xA: // ANNN set index register I
            I_reg = insty.getLastThreeNibbles();
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
        default:
            printf("Unhandled opcode %#6x\n", insty.whole);
    }
}
