#include "game_engine_port_esp32_idf.h"
#include <thread>

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
    return true;
}

}
