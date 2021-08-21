#include <thread>
#include <sstream>
#include <esp_log.h>
#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"
#include "wifi_station.h"
#include "http_weather.h"
#include "websocket.h"
#include "RpcCore.hpp"

const static char *TAG = "MAIN";
static WebsocketClient client;
static std::shared_ptr<RpcCore::Rpc> rpc;

static void start_rpc_task() {
    using namespace RpcCore;

    auto connection = std::make_shared<Connection>([&](std::string package) {
        client.send(package.data(), package.length());
    });
    client.onReceivedData = [connection](std::string package) {
        connection->onRecvPacket(package);
    };
    client.start("ws://192.168.0.110:3000");

    // 创建Rpc 收发消息
    rpc = Rpc::create(connection);
    rpc->setTimer([](uint32_t ms, const Rpc::TimeoutCb &cb) {
        // todo
    });
}

extern "C"
void app_main() {
    static std::string the_ip;
    static std::string temperature;
    static std::string weather;
    static std::string update_time;

    wifi_init_sta([](const char* ip){
        the_ip = std::string(ip);
    });

    start_rpc_task();
    oled.begin();
    rpc->subscribe<RpcCore::Bianry>("img", [](const RpcCore::Bianry& img) {
        static Screen screen;
        screen.onClear();
        screen.drawBitmap(0, 0, reinterpret_cast<const uint8_t*>(img.data()), 128, 64, 1);
        screen.onDraw();
    });

    return;

    std::thread thread_weather([]{
        for(;;) {
            std::string http_result = http_get_weather();
            if (http_result.empty()) {
                sleep(1);
                continue;
            }
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
