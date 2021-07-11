#include "OLED.h"
#include <esp_log.h>
#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"
#include "u8g2.h"

#define PIN_SDA     GPIO_NUM_4
#define PIN_SCL     GPIO_NUM_15
#define PIN_RESET   GPIO_NUM_16

static const char *TAG = "ssd1306";

OLED::OLED() {
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = PIN_SDA;
    u8g2_esp32_hal.scl = PIN_SCL;
    u8g2_esp32_hal.reset = PIN_RESET;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
            &u8g2,
            U8G2_R0,
            u8g2_esp32_i2c_byte_cb,
            u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
    u8x8_SetI2CAddress(&u8g2.u8x8,0x3c);
    ESP_LOGD(TAG, "u8g2_InitDisplay");
    u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
    ESP_LOGI(TAG, "u8g2_SetPowerSave");
    u8g2_SetPowerSave(&u8g2, 0); // wake up display
}

void OLED::demoShow() {
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawBox(&u8g2, 0, 26, 80,6);
    u8g2_DrawFrame(&u8g2, 0,26,100,6);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);
    u8g2_DrawStr(&u8g2, 2,17,"Hello World!");
    u8g2_SendBuffer(&u8g2);
    ESP_LOGI(TAG, "All done!");
}

OLED oled;

