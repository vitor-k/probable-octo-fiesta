
#pragma once

struct SDL_Window;
struct SDL_Surface;

struct Color{
    char r;
    char g;
    char b;
};

class SDL_impl{
public:
    SDL_impl();
    ~SDL_impl();
    
    void PollEvents();
    void Present();
    bool IsOpen();
private:
    Color bg;
    Color foreground;
    Color background;

    //The window we'll be rendering to
    SDL_Window* window = nullptr;

    SDL_Surface* contentSurface;

    bool is_open;
};
