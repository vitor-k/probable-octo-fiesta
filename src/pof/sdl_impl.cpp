#include <map>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

#include <fmt/core.h>

#include "sdl_impl.h"
#include "core/chip8.h"

namespace {
    //Screen dimension constants
    constexpr int SCREEN_WIDTH = 640;
    constexpr int SCREEN_HEIGHT = 480;

    static const std::map<SDL_Scancode, uint8_t> keymap {
        {SDL_SCANCODE_1, 0x1},
        {SDL_SCANCODE_2, 0x2},
        {SDL_SCANCODE_3, 0x3},
        {SDL_SCANCODE_4, 0xC},
        {SDL_SCANCODE_Q, 0x4},
        {SDL_SCANCODE_W, 0x5},
        {SDL_SCANCODE_E, 0x6},
        {SDL_SCANCODE_R, 0xD},
        {SDL_SCANCODE_A, 0x7},
        {SDL_SCANCODE_S, 0x8},
        {SDL_SCANCODE_D, 0x9},
        {SDL_SCANCODE_F, 0xE},
        {SDL_SCANCODE_Z, 0xA},
        {SDL_SCANCODE_X, 0x0},
        {SDL_SCANCODE_C, 0xB},
        {SDL_SCANCODE_V, 0xF},
    };
} // Anonymous namespace

SDL_impl::SDL_impl(Chip8& chip) : chip(chip){
    background.r = 0;
    background.g = 0;
    background.b = 0;
    foreground.r = 0x00;
    foreground.g = 0x8F;
    foreground.b = 0x11;
    bg.r = 0xE0;
    bg.g = 0xE0;
    bg.b = 0xE0;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        fmt::print( "SDL could not initialize! SDL_Error: {}\n", SDL_GetError() );
    }
    //Create window
    window = SDL_CreateWindow( "POF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if( window == NULL ) {
        fmt::print( "Window could not be created! SDL_Error: {}\n", SDL_GetError() );
    }
    else {
        is_open = true;
        SDL_SetWindowMinimumSize(window, nWidth, nHeight);

        SDL_GLContext context = SDL_GL_CreateContext(window);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            fmt::print("Failed to initialize OpenGL context\n");
        }
        fmt::print("OpenGL Version {}.{} loaded\n", GLVersion.major, GLVersion.minor);
        fmt::print("{}\n", glGetString(GL_VERSION));
        fmt::print("{}\n", glGetString(GL_VENDOR));

        SDL_GL_DeleteContext(context);

        //Get window surface
        SDL_Surface * screenSurface = SDL_GetWindowSurface( window );

        contentSurface = SDL_CreateRGBSurface(0, nWidth, nHeight,32,0,0,0,0);

        //Fill the surface white
        SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
        
        //Update the surface
        SDL_UpdateWindowSurface( window );
    }
}

void SDL_impl::Present() {
    while (IsOpen()) {
        if(chip.isFrameDirty()) {
            chip.frame_mutex.lock();
            SDL_LockSurface(contentSurface);
            uint32_t *underlying_buffer = static_cast<uint32_t*>(contentSurface->pixels);
            for(int i=0; i < nHeight; i++){
                for(int j=0; j < nWidth; j++){
                    if(chip.frameAt(j,i)) {
                        underlying_buffer[i*nWidth + j] = SDL_MapRGB(contentSurface->format, foreground.r, foreground.g, foreground.b);
                    }
                    else {
                        underlying_buffer[i*nWidth + j] = SDL_MapRGB(contentSurface->format, background.r, background.g, background.b);
                    }
                }
            }
            SDL_UnlockSurface(contentSurface);
            chip.frame_mutex.unlock();

            chip.clearDirty();
        }

        SDL_Surface *const screenSurface = SDL_GetWindowSurface( window );
        SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, bg.r, bg.g, bg.b ) );

        // Fit the content to the window
        double aspect = screenSurface->w / (double)screenSurface->h;
        int width, height;
        if(aspect > 2.0) {
            height = screenSurface->h;
            width = 2 * height;
        }
        else {
            width = screenSurface->w;
            height = width / 2;
        }
        // Fixed on the top left
        SDL_Rect rect{};
        rect.x = 0;
        rect.y = 0;
        rect.w = width;
        rect.h = height;
        SDL_BlitScaled(contentSurface, NULL, screenSurface, &rect);
        SDL_UpdateWindowSurface( window );
    }
}

void SDL_impl::PollEvents() {
    SDL_Event event;

    // SDL_PollEvent returns 0 when there are no more events in the event queue
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if(keymap.count(event.key.keysym.scancode) && !event.key.repeat) {
                chip.setKey(keymap.at(event.key.keysym.scancode), (event.key.state == SDL_PRESSED));
            }
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                is_open = false;
            }
            break;
        case SDL_QUIT:
            is_open = false;
            break;
        default:
            break;
        }
    }

}

bool SDL_impl::IsOpen() {
    return is_open && chip.isRunning();
}

SDL_impl::~SDL_impl(){
    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();
}
