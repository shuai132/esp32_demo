#include <cmath>
#include "game_engine_port.h"
#include "oled/OLED.h"

struct Screen : public ge::Canvas {
    Screen() {
        u8g2_SetFont(&oled.u8g2, u8g2_font_6x10_tf);
    }

    void onClear() override {
        u8g2_ClearBuffer(&oled.u8g2);
    }

    void onDraw() override {
        u8g2_SendBuffer(&oled.u8g2);
    }

    void drawBitmap(uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t w, uint16_t h, uint16_t color) override {
        u8g2_DrawBitmap(&oled.u8g2, x, y, ceil((double)w / 8), h, bitmap);
    }

    size_t drawBuffer(uint16_t x, uint16_t y, const char *buffer, size_t len) override {
        return u8g2_DrawStr(&oled.u8g2, x, y+10, buffer);
    }
};
