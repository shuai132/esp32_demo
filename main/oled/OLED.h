#pragma once

#include "u8g2_esp32_hal.h"

struct OLED {
    OLED();
    void demoShow();
    u8g2_t u8g2;
};

extern OLED oled;
