#pragma once

#include <cstdint>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>

class DisplayManager {
public:
    using RenderCallback = void (*)(void* context);

    enum class Font : uint8_t {
        NORMAL,
        BOLD,
        SMALL
    };

    DisplayManager();

    void init(bool initial_boot);
    void render_frame(
        RenderCallback callback, void* context, bool partial = true,
        int16_t x = 0, int16_t y = 0, int16_t w = 0, int16_t h = 0
    );

    void draw_text(
        int16_t x, int16_t y,
        const char* text, uint8_t size = 1,
        bool invert = false, Font font = Font::NORMAL
    );

    void draw_bitmap(
        int16_t x, int16_t y, const uint8_t* bitmap,
        int16_t w, int16_t h
    );

    void get_text_bounds(
        const char* text, int16_t x, int16_t y,
        int16_t* out_x = nullptr, int16_t* out_y = nullptr,
        uint16_t* out_w = nullptr, uint16_t* out_h = nullptr,
        uint8_t size = 1
    );
    
    void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void draw_highlight(int16_t x, int16_t y, int16_t w, int16_t h);
    void draw_circle(int16_t x, int16_t y, int16_t r = 2);
    
    int16_t width() const {return display_.width();}
    int16_t height() const {return display_.height();}

private:
    GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display_;
};