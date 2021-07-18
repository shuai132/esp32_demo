#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"

extern "C"
void app_main() {
    oled.begin();
    ScreenConfig c{};
    c.SCREEN_WIDTH = 128;
    c.SCREEN_HEIGHT = 32;
    c.PER_CHAR_WIDTH = 6;
    c.fps = 120;
    c.canvas = new Screen;
    start_game(&c);
}
