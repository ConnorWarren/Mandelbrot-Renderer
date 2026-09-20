#include <complex>
#include <memory>
#include <SDL3/SDL.h>
#include <iostream>

// Squared escape radius
const double ESCAPE_RADIUS = 4;
const int MAX_ITERATIONS = 1000;

using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

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
    TexturePtr texture;
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
                    handleWindowResized(event);
                    break;
            }
        }
        return true;
    }
    void handleWindowResized(SDL_Event event) {
        windowWidth = event.window.data1;
        windowHeight = event.window.data2;
        texture = createTexture();
        updateRendering = true;
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
        void *pixels = NULL;
        int pitch = 0;

        if (!SDL_LockTexture(texture.get(), nullptr, &pixels, &pitch)) {
            return;
        }
        uint32_t *pixelBuffer = (uint32_t *) pixels;

        for (int y = 0; y < windowHeight; y++) {
            for (int x = 0; x < windowWidth; x++) {
                int index = y * (pitch / sizeof(uint32_t)) + x;
                uint8_t r = pixelData[index] * 255;
                uint8_t g = pixelData[index] * 255;
                uint8_t b = pixelData[index] * 255;
                uint8_t a = 255;
                pixelBuffer[index] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        SDL_UnlockTexture(texture.get());
        SDL_RenderTexture(renderer.get(), texture.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer.get());
    }
    TexturePtr createTexture() {
        return TexturePtr{
            SDL_CreateTexture(
                renderer.get(),
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                windowWidth, windowHeight
            ),
            SDL_DestroyTexture
        };
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
    MandelbrotWindow() : texture{createTexture()} {}
};

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }
    MandelbrotWindow window;
    window.run();
    SDL_Quit();
}

