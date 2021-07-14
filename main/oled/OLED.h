#pragma once

#include "Adafruit_GFX.h"

class OLED : public GFXcanvas1 {
public:
    using GFXcanvas1::GFXcanvas1;

    void clearDisplay();

    void display();

    void invertDisplay(bool i) override;

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    void begin();
};

extern OLED oled;
