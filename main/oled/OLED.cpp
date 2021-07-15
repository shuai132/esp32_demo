/**
 * SSD1306 driver for ESP32
 * modify from Adafruit_SSD1306.cpp
 */
#include "OLED.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "unity.h"

#define OLED_WIDTH              (128)
#define OLED_HEIGHT             (64)
#define OLED_I2C_SDA_GPIO       (4)
#define OLED_I2C_SCL_GPIO       (15)
#define OLED_PIN_RESET          GPIO_NUM_16
#define I2C_MASTER_NUM          I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ      800000  /*!< I2C master clock frequency */
#define ESP_SLAVE_ADDR          0x3C    /*!< ESP32 slave address, you can set any 7bit value */
#define ACK_CHECK_EN            0x1     /*!< I2C master will check ack from slave*/

// SSD1306 commands
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE  0x20
#define SSD1306_CMD_SET_COLUMN_RANGE      0x21
#define SSD1306_CMD_SET_PAGE_RANGE        0x22
#define SSD1306_CMD_SET_CHARGE_PUMP       0x8D
#define SSD1306_CMD_MIRROR_X_OFF          0xA0
#define SSD1306_CMD_MIRROR_X_ON           0xA1
#define SSD1306_CMD_INVERT_OFF            0xA6
#define SSD1306_CMD_INVERT_ON             0xA7
#define SSD1306_CMD_DISP_OFF              0xAE
#define SSD1306_CMD_DISP_ON               0xAF
#define SSD1306_CMD_MIRROR_Y_OFF          0xC0
#define SSD1306_CMD_MIRROR_Y_ON           0xC8

/// fit into the SSD1306_ naming scheme
#define SSD1306_BLACK 0   ///< Draw 'off' pixels
#define SSD1306_WHITE 1   ///< Draw 'on' pixels
#define SSD1306_INVERSE 2 ///< Invert pixels

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

#define PROGMEM

#define ssd1306_swap(a, b)                                                     \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

static const uint8_t vccstate = SSD1306_SWITCHCAPVCC;

static void ssd1306_command1(uint8_t command) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    TEST_ESP_OK(i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write_byte(cmd, 0, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write_byte(cmd, command, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_stop(cmd));
    TEST_ESP_OK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, portMAX_DELAY));
    i2c_cmd_link_delete(cmd);
}

static void ssd1306_commandList(const uint8_t* cmdList, size_t size) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    TEST_ESP_OK(i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write_byte(cmd, 0, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write(cmd, cmdList, size, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_stop(cmd));
    TEST_ESP_OK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, portMAX_DELAY));
    i2c_cmd_link_delete(cmd);
}

static void ssd1306_writeData(const uint8_t * data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    TEST_ESP_OK(i2c_master_write_byte(cmd, ( ESP_SLAVE_ADDR << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write_byte(cmd, 0x40, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_write(cmd, data, len, ACK_CHECK_EN));
    TEST_ESP_OK(i2c_master_stop(cmd));
    TEST_ESP_OK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, portMAX_DELAY));
    i2c_cmd_link_delete(cmd);
}

static i2c_config_t i2c_master_init() {
    i2c_config_t conf_master = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = OLED_I2C_SDA_GPIO,
            .scl_io_num = OLED_I2C_SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .clk_flags = 0,
    };
    conf_master.master.clk_speed = I2C_MASTER_FREQ_HZ;
    return conf_master;
}

static esp_err_t ssd1306_reset() {
    gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << OLED_PIN_RESET,
            .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t ret = gpio_config(&io_conf);
    // perform hardware reset
    gpio_set_level(OLED_PIN_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(OLED_PIN_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(OLED_PIN_RESET, 1);
    return ret;
}

void OLED::begin() {
    i2c_config_t conf_master = i2c_master_init();
    TEST_ESP_OK(i2c_param_config(I2C_MASTER_NUM, &conf_master));
    TEST_ESP_OK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER,
                                   I2C_MASTER_RX_BUF_DISABLE,
                                   I2C_MASTER_TX_BUF_DISABLE, 0));

    TEST_ESP_OK(ssd1306_reset());
    // Init sequence
    static const uint8_t PROGMEM init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
            SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
            0x80, // the suggested ratio 0x80
            SSD1306_SETMULTIPLEX}; // 0xA8
    ssd1306_commandList(init1, sizeof(init1));
    ssd1306_command1(HEIGHT - 1);

    static const uint8_t PROGMEM init2[] = {SSD1306_SETDISPLAYOFFSET, // 0xD3
            0x0,                      // no offset
            SSD1306_SETSTARTLINE | 0x0, // line #0
            SSD1306_CHARGEPUMP};        // 0x8D
    ssd1306_commandList(init2, sizeof(init2));

    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    static const uint8_t PROGMEM init3[] = {SSD1306_MEMORYMODE, // 0x20
            0x00, // 0x0 act like ks0108
            SSD1306_SEGREMAP | 0x1,
            SSD1306_COMSCANDEC};
    ssd1306_commandList(init3, sizeof(init3));

    uint8_t comPins = 0x02;
    auto contrast = 0x8F;

    if ((WIDTH == 128) && (HEIGHT == 32)) {
        comPins = 0x02;
        contrast = 0x8F;
    } else if ((WIDTH == 128) && (HEIGHT == 64)) {
        comPins = 0x12;
        contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
    } else if ((WIDTH == 96) && (HEIGHT == 16)) {
        comPins = 0x2; // ada x12
        contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF;
    } else {
        // Other screen varieties -- TBD
    }

    ssd1306_command1(SSD1306_SETCOMPINS);
    ssd1306_command1(comPins);
    ssd1306_command1(SSD1306_SETCONTRAST);
    ssd1306_command1(contrast);

    ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    static const uint8_t PROGMEM init5[] = {
            SSD1306_SETVCOMDETECT, // 0xDB
            0x40,
            SSD1306_DISPLAYALLON_RESUME, // 0xA4
            SSD1306_NORMALDISPLAY,       // 0xA6
            SSD1306_DEACTIVATE_SCROLL,
            SSD1306_DISPLAYON}; // Main screen turn on
    ssd1306_commandList(init5, sizeof(init5));
}

void OLED::clearDisplay() {
    this->fillScreen(0);
}

void OLED::display() {
    static const uint8_t PROGMEM dlist1[] = {
            SSD1306_PAGEADDR,
            0,                      // Page start address
            0xFF,                   // Page end (not really, but works here)
            SSD1306_COLUMNADDR, 0}; // Column start address
    ssd1306_commandList(dlist1, sizeof(dlist1));
    ssd1306_command1(WIDTH - 1); // Column end address
    ssd1306_writeData(getBuffer(), WIDTH * HEIGHT / 8);
}

void OLED::invertDisplay(bool i) {
    ssd1306_command1(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

void OLED::drawPixel(int16_t x, int16_t y, uint16_t color) {
    uint8_t *buffer = getBuffer();
    if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
        // Pixel is in-bounds. Rotate coordinates if needed.
        switch (getRotation()) {
            case 1:
                ssd1306_swap(x, y);
                x = WIDTH - x - 1;
                break;
            case 2:
                x = WIDTH - x - 1;
                y = HEIGHT - y - 1;
                break;
            case 3:
                ssd1306_swap(x, y);
                y = HEIGHT - y - 1;
                break;
        }
        switch (color) {
            case SSD1306_WHITE:
                buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
                break;
            case SSD1306_BLACK:
                buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
                break;
            case SSD1306_INVERSE:
                buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
                break;
        }
    }
}

OLED oled(OLED_WIDTH, OLED_HEIGHT);
