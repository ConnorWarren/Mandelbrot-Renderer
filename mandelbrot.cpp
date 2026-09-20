#include <complex>
#include <memory>
#include <SDL3/SDL.h>
#include <iostream>

// Squared escape radius
const double ESCAPE_RADIUS = 4;
const int MAX_ITERATIONS = 1000;

using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

bool insideMandelbrot(std::complex<double> c) {
    int i = 0;
    std::complex<double> z = {0, 0};
    while (i < MAX_ITERATIONS) {
        if (std::norm(z) > ESCAPE_RADIUS)
            return false;
        z = z * z + c;
        i++;
    }
    return true;
}

class MandelbrotRegion {
private:
    std::complex<double> m_topLeft;
    std::complex<double> m_bottomRight;
public:
    MandelbrotRegion() : m_topLeft{-2.0, 1.0}, m_bottomRight{1.0, -1.0} {}
    MandelbrotRegion(std::complex<double> topLeft, std::complex<double> bottomRight) 
        : m_topLeft{topLeft}, m_bottomRight{bottomRight} {}
    std::complex<double> topLeft() {
        return m_topLeft;
    }
    std::complex<double> bottomRight() {
        return m_bottomRight;
    }
    double realRange() {
        return m_bottomRight.real() - m_topLeft.real();
    }
    double imagRange() {
        return m_topLeft.imag() - m_bottomRight.imag();
    }
};

class MandelbrotWindow {
private:
    int windowWidth = 600;
    int windowHeight = 400;
    WindowPtr window{
        SDL_CreateWindow(
            "Mandelbrot Renderer",
            windowWidth, windowHeight, SDL_WINDOW_RESIZABLE
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
    std::unique_ptr<int[]> pixelData;
    bool running;
    // Keeps track of visible rendering updates
    bool updateRendering;
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
                case SDL_EVENT_WINDOW_RESIZED:
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                    updateRendering = true;
                    break;
            }
        }
        return true;
    }
    void computeSet() {
        int resolution = windowWidth * windowHeight;
        pixelData = std::make_unique<int[]>(resolution);
        for (int i = 0; i < resolution; i++) {
            int row = i / windowWidth;
            int col = i % windowWidth;
            double real = region.realRange() / windowWidth * col + region.topLeft().real();
            double imag = region.imagRange() / windowHeight * row + region.bottomRight().imag();
            std::complex<double> point{
                real, imag
            };
            pixelData[i] = (int) insideMandelbrot(point);
       }
    }
    void render() {
        SDL_RenderClear(renderer.get());
        for (int i = 0; i < windowWidth * windowHeight; i++) {
            if (pixelData[i])
                SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
            else
                SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
            int row = i / windowWidth;
            int col = i % windowWidth;
            SDL_RenderPoint(renderer.get(), row, col);
        }
        SDL_RenderPresent(renderer.get());
    }
public:
    void run(MandelbrotRegion region = MandelbrotRegion()) {
        running = true;
        updateRendering = true;
        this->region = region;
        while (running) {
            if (!handleEvents())
                // user quit, don't render next frame.
                break;
            if (updateRendering) {
                computeSet();
                render();
                updateRendering = false;
            }
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

