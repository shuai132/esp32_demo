#include <thread>
#include <sstream>
#include <utility>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"
#include "wifi_station.h"
#include "http_weather.h"
#include "websocket.h"
#include "RpcCore.hpp"
#include "ArduinoJson.hpp"
#include "asio.hpp"
#include "utils.h"
#include "esp_pthread.h"
#include "smartconfig.h"

const static char *TAG = "MAIN";

static void start_rpc_task() {
    using namespace RpcCore;

    static WebsocketClient client;
    static std::shared_ptr<RpcCore::Rpc> rpc;
    static asio::io_context context;

    auto connection = std::make_shared<Connection>([&](std::string package) {
        client.send(package.data(), package.length());
    });
    client.onReceivedData = [connection](std::string package) {
        connection->onRecvPackage(std::move(package));
    };
    client.start("ws://192.168.0.109:3000");

    rpc = Rpc::create(connection);
    rpc->setTimer([&](uint32_t ms, Rpc::TimeoutCb cb) {
        utils::steady_timer(&context, std::move(cb), ms);
    });
    oled.begin();
    rpc->subscribe<RpcCore::Binary>("img", [](const RpcCore::Binary& img) {
        static Screen screen;
        screen.onClear();
        screen.drawBitmap(0, 0, reinterpret_cast<const uint8_t*>(img.data()), 128, 64, 1);
        screen.onDraw();
    });
    asio::io_context::work work(context);
    context.run();
}

extern "C"
void app_main() {
    static std::string the_ip;
    static std::string temperature;
    static std::string weather;
    static std::string update_time;

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    start_smartconfig_task([](const WiFiInfo& info) {
        ESP_LOGI(TAG, "WiFiInfo:%s %s", info.ssid.c_str(), info.passwd.c_str());
    }, [](ConnectState state) {
        ESP_LOGI(TAG, "ConnectState:%d", (int)state);
    });
    return;

    esp_pthread_cfg_t cfg{1024*40, 5, false, "rpc_task", tskNO_AFFINITY};
    esp_pthread_set_cfg(&cfg);
    std::thread(start_rpc_task).detach();

    esp_pthread_cfg_t cfg2{1024*40, 5, false, "weather_task", tskNO_AFFINITY};
    esp_pthread_set_cfg(&cfg2);
    std::thread([]{
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
    }).detach();

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
