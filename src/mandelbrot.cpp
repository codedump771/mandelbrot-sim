#include <SDL3/SDL.h>
#include <omp.h>
#include <print>
#include <complex>
#include <cstdint>
#include <vector>

class MandelbrotSim {
private:
    static constexpr uint32_t WINDOW_WIDTH { 1200 };
    static constexpr uint32_t WINDOW_HEIGHT { 900 };
    
    static constexpr double ASPECT_RATIO { 
        static_cast<double>(WINDOW_WIDTH) / static_cast<double>(WINDOW_HEIGHT) 
    };
    
    double center_x_ { 0.0 };
    double center_y_ { 0.0 };
    double scale_x_ { 5.0 / ASPECT_RATIO };
    double scale_y_ { 3.0 };

    static constexpr double MOVE_SPEED { 0.3 };
    static constexpr double ZOOM_SPEED { 1.3 };
    
    static constexpr uint32_t MAX_ITERATIONS { 150 };

    SDL_Window* window_ { nullptr };
    SDL_Renderer* renderer_ { nullptr };
    SDL_Texture* texture_ { nullptr };
    SDL_Event event_;

    std::vector<uint32_t> pixels_;
    double x_dis { 0.0 };
    double y_dis { 0.0 };
    double zoom { 1.0 };
    bool running_ { true };

    uint64_t rgba_to_hex(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {

        return (static_cast<uint32_t>(a) << 24) +
               (static_cast<uint32_t>(b) << 16) +
               (static_cast<uint32_t>(g) << 8) +
                static_cast<uint32_t>(r);
    }

    uint64_t hsv_to_hex(float h, float s, float v) {
        float r, g, b;

        int32_t i = std::floor(h * 6);
        float f = h * 6 - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);

        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }
        return rgba_to_hex(r*255, g*255, b*255, 255);
    }

    std::complex<double> pixel_to_plane(uint32_t x, uint32_t y) {
        
        double min_x = x_dis + center_x_ - scale_x_ / 2.0 * zoom;
        double max_x = x_dis + center_x_ + scale_x_ / 2.0 * zoom;
        double min_y = y_dis + center_y_ + scale_y_ / 2.0 * zoom;
        double max_y = y_dis + center_y_ - scale_y_ / 2.0 * zoom;
        
        double r = x * (max_x - min_x) / WINDOW_WIDTH + min_x;
        double i = y * (max_y - min_y) / WINDOW_HEIGHT + min_y;

        return std::complex<double>(r, i);
    }

public:
    uint64_t compute_escape(std::complex<double> c) {
        std::complex<double> z(0, 0);

        uint32_t i;
        for (i = 0; i < MAX_ITERATIONS; i++) {
            if (std::norm(z) > 4.0) {
                float color = i + 1 - std::log(std::log(std::abs(z))) / std::log(2.0);
                double norm_color = color / MAX_ITERATIONS;
                return rgba_to_hex(0, 0, 255*norm_color, 255);
            }

            z = z * z + c;
        }

        return 0;
    }

    void init() {
        if (!SDL_Init(SDL_INIT_VIDEO))
            std::println("[ERROR] {}", SDL_GetError());

        SDL_CreateWindowAndRenderer("Mandelbrot Simulation", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window_, &renderer_);

        if (!window_ || !renderer_)
            std::println("[ERROR] {}", SDL_GetError());

        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!texture_) {
            std::println("[ERROR] {}", SDL_GetError());
        }

        pixels_.reserve(WINDOW_WIDTH*WINDOW_HEIGHT);
    }

    void plot_set() {

        SDL_RenderClear(renderer_);

        #pragma omp parallel for collapse(2)
        for (int32_t y = 0; y < WINDOW_HEIGHT; y++) {
            for (int32_t x = 0; x < WINDOW_WIDTH; x++) {
                pixels_[x + y * WINDOW_WIDTH] = compute_escape(pixel_to_plane(x, y));
            }
        }    
        
        SDL_UpdateTexture(texture_, nullptr, pixels_.data(), WINDOW_WIDTH * sizeof(uint32_t));

        if (!texture_) {
            std::println("[ERROR] {}", SDL_GetError());
        }
        
        SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
        SDL_RenderPresent(renderer_);
    }
    
    void loop() {
        while (running_) {
            handle_input();
            plot_set();
        }
    }

    void quit() {
        SDL_DestroyTexture(texture_);
        SDL_DestroyWindow(window_);
        SDL_DestroyRenderer(renderer_);
        SDL_Quit();
        std::exit(0);
    }

    void handle_input() {
        while(SDL_PollEvent(&event_)) {
            if (event_.type == SDL_EVENT_QUIT)
                quit();

            if (event_.type == SDL_EVENT_KEY_DOWN) {
                switch(event_.key.key) {
                    case SDLK_D: x_dis += MOVE_SPEED * zoom; break;
                    case SDLK_A: x_dis -= MOVE_SPEED * zoom; break;
                    case SDLK_W: y_dis += MOVE_SPEED * zoom; break;
                    case SDLK_S: y_dis -= MOVE_SPEED * zoom; break;
                }
            } 

            if (event_.type == SDL_EVENT_MOUSE_WHEEL) {
                if (event_.wheel.y > 0.0f) {
                    zoom /= ZOOM_SPEED;
                }
                else if (event_.wheel.y < 0.0f) {
                    zoom *= ZOOM_SPEED;
                }
            }
        }
    }

    ~MandelbrotSim() { quit(); }
};

int main() {
    MandelbrotSim sim;

    sim.init();
    sim.loop();
}
