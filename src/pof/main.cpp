#include <iostream>
#include <stack>
#include <fstream>
#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

//Native screen dimensions
constexpr unsigned int nWidth = 64;
constexpr unsigned int nHeight = 32;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

int main(int argc, char* args[]) {
    //The window we'll be rendering to
    SDL_Window* window = NULL;
    
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else {
        //Create window
        window = SDL_CreateWindow( "POF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if( window == NULL ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else {
            SDL_GL_CreateContext(window);

            if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
                printf("Failed to initialize OpenGL context\n");
                return -1;
            }
            printf("OpenGL Version %d.%d loaded\n", GLVersion.major, GLVersion.minor);
            printf("%s\n", glGetString(GL_VERSION));
            printf("%s\n", glGetString(GL_VENDOR));

            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );

            //Wait two seconds
            SDL_Delay( 2000 );
        }
    }
    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}
