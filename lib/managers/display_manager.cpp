#include "display_manager.h"
#include "config.h"
#include <SPI.h>
#include <cstring>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define ENABLE_GxEPD2_GFX 0

namespace {
    constexpr uint8_t HIGHLIGHT_PADDING_X = 6;
    constexpr uint8_t HIGHLIGHT_PADDING_Y = 4;
    constexpr uint8_t HIGHLIGHT_PADDING_W = 12;
    constexpr uint8_t HIGHLIGHT_PADDING_H = 8;

    void to_ascii(char* str) {
        if (!str) return;

        char* src = str;
        char* dst = str;

        while (*src) {
            uint8_t b1 = static_cast<uint8_t>(*src);

            if (b1 < 0x80) {
                *dst++ = *src++;
            } else if (b1 == 0xC3) {
                src++;
                uint8_t b2 = static_cast<uint8_t>(*src++);
                switch (b2) {
                    case 0xA1: *dst++ = 'a'; break;
                    case 0xA9: *dst++ = 'e'; break;
                    case 0xAD: *dst++ = 'i'; break;
                    case 0xB3: *dst++ = 'o'; break;
                    case 0xB6: *dst++ = 'o'; break;
                    case 0xBA: *dst++ = 'u'; break;
                    case 0xBC: *dst++ = 'u'; break;
                    case 0x81: *dst++ = 'A'; break;
                    case 0x89: *dst++ = 'E'; break;
                    case 0x8D: *dst++ = 'I'; break;
                    case 0x93: *dst++ = 'O'; break;
                    case 0x96: *dst++ = 'O'; break;
                    case 0x9A: *dst++ = 'U'; break;
                    case 0x9C: *dst++ = 'U'; break;
                    default:   *dst++ = '?'; break;
                }
            } else if (b1 == 0xC5) {
                src++;
                uint8_t b2 = static_cast<uint8_t>(*src++);
                switch (b2) {
                    case 0x91: *dst++ = 'o'; break;
                    case 0x90: *dst++ = 'O'; break;
                    case 0xB1: *dst++ = 'u'; break;
                    case 0xB0: *dst++ = 'U'; break;
                    default:   *dst++ = '?'; break;
                }
            } else {
                src++;
            }
        }
        *dst = '\0';
    }
}

DisplayManager::DisplayManager()
    : display_(GxEPD2_290_BS(Pins::SPI_CS, Pins::SPI_DC, Pins::SPI_RES, Pins::SPI_BUSY)) {}

void DisplayManager::init(bool initial_boot) {
    pinMode(Pins::SPI_CS, OUTPUT);
    pinMode(Pins::SPI_DC, OUTPUT);
    pinMode(Pins::SPI_RES, OUTPUT);
    pinMode(Pins::SPI_BUSY, INPUT);

    digitalWrite(Pins::SPI_CS, HIGH);
    digitalWrite(Pins::SPI_RES, HIGH);

    SPI.begin(Pins::SPI_SCL, -1, Pins::SPI_SDA, Pins::SPI_CS);

    display_.init(TimingConfig::BAUD_RATE, initial_boot, 50, false);
    display_.setRotation(1);
    display_.setTextColor(GxEPD_BLACK);

    if (initial_boot) {
        display_.setFullWindow();
        do {
            display_.fillScreen(GxEPD_WHITE);
            display_.setFont(&FreeSansBold9pt7b);
            display_.setCursor(96, 71);
            display_.print("Initializing...");
        } while (display_.nextPage());
    }
}

void DisplayManager::render_frame(RenderCallback callback, void* context, bool partial, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (partial) {
        display_.setPartialWindow(x, y, w, h);
    } else {
        display_.setFullWindow();
    }

    display_.firstPage();
    do {
        if (callback != nullptr) {
            callback(context);
        }
    } while (display_.nextPage());

    display_.hibernate();
}

void DisplayManager::draw_text(int16_t x, int16_t y, const char* text, uint8_t size, bool invert, Font font) {
    if (!text) return;

    char text_buf[64];
    strncpy(text_buf, text, sizeof(text_buf) - 1);
    text_buf[sizeof(text_buf) - 1] = '\0';
    
    to_ascii(text_buf);
    
    switch (font) {
        case Font::BOLD: display_.setFont(&FreeSansBold9pt7b); break;
        case Font::SMALL: display_.setFont(nullptr); break;
        case Font::NORMAL: display_.setFont(&FreeSans9pt7b); break;
        default: break;
    }
    display_.setCursor(x, y);
    display_.setTextSize(size);
    display_.setTextColor(invert ? GxEPD_WHITE : GxEPD_BLACK, invert ? GxEPD_BLACK : GxEPD_WHITE);
    display_.print(text_buf);
}

void DisplayManager::draw_highlight(int16_t x, int16_t y, int16_t w, int16_t h) {
    display_.fillRect(x - HIGHLIGHT_PADDING_X, y - HIGHLIGHT_PADDING_Y, w + HIGHLIGHT_PADDING_W, h + HIGHLIGHT_PADDING_H, GxEPD_BLACK);
}

void DisplayManager::draw_bitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h) {
    display_.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
}

void DisplayManager::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    display_.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
}

void DisplayManager::draw_circle(int16_t x, int16_t y, int16_t r) {
    display_.drawCircle(x, y, r, GxEPD_BLACK);
}

void DisplayManager::get_text_bounds(const char* text, int16_t x, int16_t y, int16_t* out_x, int16_t* out_y, uint16_t* out_w, uint16_t* out_h, uint8_t size) {
    if (!text) return;
    display_.setTextSize(size);
    display_.getTextBounds(text, x, y, out_x, out_y, out_w, out_h);
}