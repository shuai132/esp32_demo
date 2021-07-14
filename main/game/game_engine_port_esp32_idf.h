#include <cmath>
#include "game_engine_port.h"
#include "oled/OLED.h"

struct Screen : public ge::Canvas {
    Screen();

    void onClear() override {
        oled.clearDisplay();
    }

    void onDraw() override {
        oled.display();
    }

    void drawBitmap(uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t w, uint16_t h, uint16_t color) override {
        oled.drawBitmap(x, y, bitmap, w, h, color);
    }

    size_t drawBuffer(uint16_t x, uint16_t y, const char *buffer, size_t len) override {
        oled.setCursor(x, y);
        oled.setTextColor(1);
        return oled.write((const uint8_t*) buffer, len);
    }
};
