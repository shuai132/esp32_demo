#include "OLED.h"

#define OLED_I2C_SDA_GPIO       (GPIO_NUM_4)
#define OLED_I2C_SCL_GPIO       (GPIO_NUM_15)
#define OLED_PIN_RESET          (GPIO_NUM_16)
#define I2C_MASTER_NUM          I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ      800000  /*!< I2C master clock frequency */
#define I2C_SLAVE_ADDR          0x3C    /*!< ESP32 slave address, you can set any 7bit value */

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT,
                      I2C_MASTER_NUM, I2C_SLAVE_ADDR, I2C_MASTER_FREQ_HZ,
                      OLED_I2C_SDA_GPIO, OLED_I2C_SCL_GPIO, OLED_PIN_RESET);
