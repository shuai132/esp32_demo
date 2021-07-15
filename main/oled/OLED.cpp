#include "OLED.h"

#define OLED_WIDTH              (128)
#define OLED_HEIGHT             (64)
#define OLED_I2C_SDA_GPIO       (4)
#define OLED_I2C_SCL_GPIO       (15)
#define OLED_PIN_RESET          (16)
#define I2C_MASTER_NUM          0       /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ      800000  /*!< I2C master clock frequency */
#define I2C_SLAVE_ADDR          0x3C    /*!< ESP32 slave address, you can set any 7bit value */

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT,
                      I2C_MASTER_NUM, I2C_SLAVE_ADDR, I2C_MASTER_FREQ_HZ,
                      OLED_I2C_SDA_GPIO, OLED_I2C_SCL_GPIO, OLED_PIN_RESET);
