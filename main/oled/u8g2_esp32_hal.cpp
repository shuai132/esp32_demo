#include <thread>
#include "u8g2_esp32_hal.h"

static const unsigned int I2C_TIMEOUT_MS = 1000;

static i2c_cmd_handle_t    handle_i2c;
static u8g2_esp32_hal_t    u8g2_esp32_hal;

void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param) {
    u8g2_esp32_hal = u8g2_esp32_hal_param;
}

uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_BYTE_INIT: {
            i2c_config_t conf;
            conf.mode = I2C_MODE_MASTER;
            conf.sda_io_num = u8g2_esp32_hal.sda;
            conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
            conf.scl_io_num = u8g2_esp32_hal.scl;
            conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
            conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
            conf.clk_flags = 0;
            ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
            ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
            break;
        }

        case U8X8_MSG_BYTE_SEND: {
            uint8_t* data_ptr = (uint8_t*)arg_ptr;
            while( arg_int > 0 ) {
                ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, *data_ptr, ACK_CHECK_EN));
                data_ptr++;
                arg_int--;
            }
            break;
        }

        case U8X8_MSG_BYTE_START_TRANSFER: {
            uint8_t i2c_address = u8x8_GetI2CAddress(u8x8);
            handle_i2c = i2c_cmd_link_create();
            ESP_ERROR_CHECK(i2c_master_start(handle_i2c));
            ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, i2c_address<<1 | I2C_MASTER_WRITE, ACK_CHECK_EN));
            break;
        }

        case U8X8_MSG_BYTE_END_TRANSFER: {
            ESP_ERROR_CHECK(i2c_master_stop(handle_i2c));
            ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, handle_i2c, I2C_TIMEOUT_MS / portTICK_RATE_MS));
            i2c_cmd_link_delete(handle_i2c);
            break;
        }
    }
    return 0;
}

uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT: {
            uint64_t bitmask = 0;
            if (u8g2_esp32_hal.reset != GPIO_NUM_NC) {
                bitmask = bitmask | (1ull<<u8g2_esp32_hal.reset);
            }
            if (bitmask==0) {
                break;
            }
            gpio_config_t gpioConfig;
            gpioConfig.pin_bit_mask = bitmask;
            gpioConfig.mode         = GPIO_MODE_OUTPUT;
            gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
            gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
            gpioConfig.intr_type    = GPIO_INTR_DISABLE;
            gpio_config(&gpioConfig);
            break;
        }
        case U8X8_MSG_GPIO_RESET:
            if (u8g2_esp32_hal.reset != GPIO_NUM_NC) {
                gpio_set_level(u8g2_esp32_hal.reset, arg_int);
            }
            break;
        case U8X8_MSG_GPIO_I2C_CLOCK:
            if (u8g2_esp32_hal.scl != GPIO_NUM_NC) {
                gpio_set_level(u8g2_esp32_hal.scl, arg_int);
            }
            break;
        case U8X8_MSG_GPIO_I2C_DATA:
            if (u8g2_esp32_hal.sda != GPIO_NUM_NC) {
                gpio_set_level(u8g2_esp32_hal.sda, arg_int);
            }
            break;
        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(arg_int/portTICK_PERIOD_MS);
            break;
    }
    return 0;
}
