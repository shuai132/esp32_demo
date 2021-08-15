#include <thread>
#include <sstream>
#include <esp_log.h>
#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"
#include "wifi_station.h"
#include "http_weather.h"
#include "ArduinoJson-v6.18.3.hpp"

const static char *TAG = "MAIN";

extern "C"
void app_main() {
    static std::string the_ip;
    static std::string temperature;
    static std::string weather;
    static std::string update_time;

    std::thread thread_weather([]{
        wifi_init_sta([](const char* ip){
            the_ip = std::string(ip);
        });

        for(;;) {
            std::string http_result = http_get_weather();
            ESP_LOGI(TAG, "response:%s", http_result.c_str());

            using namespace ArduinoJson;
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, http_result);
            auto result = doc["results"][0];
            auto result_now = result["now"];
            temperature = "Temp:" + result_now["temperature"].as<std::string>();
            weather = result_now["text"].as<std::string>();
            update_time = result["last_update"].as<std::string>();

            sleep(60);
        }
    });

    auto screen = new Screen;
    screen->onBeforeDraw = [&] {
        screen->drawString(0, 8*0, the_ip.c_str());
        screen->drawString(0, 8*1, temperature.c_str());
        screen->drawString(0, 8*2, weather.c_str());
        screen->drawString(0, 8*3, update_time.c_str());
    };

    oled.begin();
    ScreenConfig c{};
    c.SCREEN_WIDTH = 128;
    c.SCREEN_HEIGHT = 64;
    c.PER_CHAR_WIDTH = 6;
    c.fps = 120;
    c.canvas = screen;
    start_game(&c);
}
