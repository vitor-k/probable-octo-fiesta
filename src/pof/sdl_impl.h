
#pragma once

struct SDL_Window;
struct SDL_Surface;

class SDL_impl{
public:
    SDL_impl();
    ~SDL_impl();
private:
    //The window we'll be rendering to
    SDL_Window* window = nullptr;

    //The surface contained by the window
    SDL_Surface* screenSurface = nullptr;
};
