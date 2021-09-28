#include <cstdio>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

#include "sdl_impl.h"
#include "chip8.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_impl::SDL_impl(){
    background.r=0;
    background.g=0;
    background.b=0;
    foreground.r=0x00;
    foreground.g=0x8F;
    foreground.b=0x11;
    bg.r = 0xE0;
    bg.g = 0xE0;
    bg.b = 0xE0;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    //Create window
    window = SDL_CreateWindow( "POF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if( window == NULL ) {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    else {
        is_open = true;
        SDL_SetWindowMinimumSize(window, nWidth, nHeight);

        SDL_GLContext context = SDL_GL_CreateContext(window);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            printf("Failed to initialize OpenGL context\n");
        }
        printf("OpenGL Version %d.%d loaded\n", GLVersion.major, GLVersion.minor);
        printf("%s\n", glGetString(GL_VERSION));
        printf("%s\n", glGetString(GL_VENDOR));

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
        if(global_chip.frame_dirty) {
            global_chip.frame_mutex.lock();
            SDL_LockSurface(contentSurface);
            uint32_t *underlying_buffer = static_cast<uint32_t*>(contentSurface->pixels);
            for(int i=0; i < nHeight; i++){
                for(int j=0; j < nWidth; j++){
                    if(global_chip.framebuffer[i*nWidth + j]) {
                        underlying_buffer[i*nWidth + j] = SDL_MapRGB(contentSurface->format, foreground.r, foreground.g, foreground.b);
                    }
                    else {
                        underlying_buffer[i*nWidth + j] = SDL_MapRGB(contentSurface->format, background.r, background.g, background.b);
                    }
                }
            }
            SDL_UnlockSurface(contentSurface);
            global_chip.frame_mutex.unlock();

            global_chip.frame_dirty = false;
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
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                break;
            case SDL_WINDOWEVENT_CLOSE:
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
    return is_open;
}

SDL_impl::~SDL_impl(){
    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();
}