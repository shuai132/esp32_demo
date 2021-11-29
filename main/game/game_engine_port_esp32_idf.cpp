#include "game_engine_port_esp32_idf.h"
#include <thread>
#include <driver/gpio.h>

#define GPIO_BUTTON GPIO_NUM_0

namespace ge {

using namespace std::chrono;

unsigned long nowMs() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
unsigned long nowUs() {
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
void delayUs(unsigned int us) {
    std::this_thread::sleep_for(microseconds(us));
}

bool checkButton() {
    return !gpio_get_level(GPIO_BUTTON);
}

}

void initButton() {
    gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << GPIO_BUTTON,
            .mode = GPIO_MODE_INPUT,
    };
    gpio_config(&io_conf);
}
