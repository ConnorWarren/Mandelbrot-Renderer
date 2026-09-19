#include <complex>
#include <memory>
#include <SDL3/SDL.h>

// Squared escape radius
const double ESCAPE_RADIUS = 4;
const int MAX_ITERATIONS = 500;

using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

bool insideMandelbrot(std::complex<double> c) {
    int i = 0;
    while (i < MAX_ITERATIONS) {
        if (std::norm(c) > ESCAPE_RADIUS)
            return false;
        i++;
    }
    return true;
}

class MandelbrotRegion {
private:
    std::complex<double> topLeft;
    std::complex<double> bottomRight;
public:
    MandelbrotRegion() : topLeft{-1.0, 1.0}, bottomRight{1.0, -1.0} {}
    MandelbrotRegion(std::complex<double> topLeft, std::complex<double> bottomRight) 
        : topLeft{topLeft}, bottomRight{bottomRight} {}
};

class MandelbrotWindow {
private:
    WindowPtr window{
        SDL_CreateWindow(
            "Mandelbrot Renderer",
            600, 400, SDL_WINDOW_RESIZABLE
        ),
        SDL_DestroyWindow
    };
    RendererPtr renderer{
        SDL_CreateRenderer(
            window.get(),
            NULL
        ),
        SDL_DestroyRenderer
    };
    MandelbrotRegion region;
    bool running;
    
    /*
    * Handle SDL events in a given frame. Returns true unless 
    * a quit event is trigged.
    */
    bool handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    return false;
            }
        }
        return true;
    }
    void render() {
        
    }
public:
    void run(MandelbrotRegion region = MandelbrotRegion()) {
        running = true;
        this->region = region;
        while (running) {
            if (!handleEvents())
                // user quit, don't render next frame.
                break;
            render();
        }
    }
};

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }
    MandelbrotWindow window;
    window.run();
    SDL_Quit();
}

