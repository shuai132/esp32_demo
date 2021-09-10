#include <thread>
#include <sstream>
#include <utility>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_pthread.h>
#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"
#include "wifi_station.h"
#include "http_weather.h"
#include "websocket.h"
#include "RpcCore.hpp"
#include "ArduinoJson.hpp"
#include "asio.hpp"
#include "utils.h"
#include "smartconfig.h"
#include "nvs.h"

const static char* TAG = "MAIN";
const static char* NS_NAME_WIFI = "wifi";
static Screen screen;
static std::string local_ip_now;

static void start_rpc_task() {
    static WebsocketClient client;
    static std::shared_ptr<RpcCore::Rpc> rpc;
    static asio::io_context context;

    using namespace RpcCore;
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
    rpc->subscribe<RpcCore::Binary>("img", [](const RpcCore::Binary& img) {
        screen.onClear();
        screen.drawBitmap(0, 0, reinterpret_cast<const uint8_t*>(img.data()), 128, 64, 1);
        screen.onDraw();
    });

    esp_pthread_cfg_t cfg{1024*40, 5, false, "rpc_task", tskNO_AFFINITY};
    esp_pthread_set_cfg(&cfg);
    std::thread([]{
        asio::io_context::work work(context);
        context.run();
    }).detach();
}

static void start_weather_task() {
    static std::string temperature;
    static std::string weather;
    static std::string update_time;

    screen.onBeforeDraw = [] {
        screen.drawString(0, 8*0, local_ip_now.c_str());
        screen.drawString(0, 8*1, temperature.c_str());
        screen.drawString(0, 8*2, weather.c_str());
        screen.drawString(0, 8*3, update_time.c_str());
    };

    esp_pthread_cfg_t cfg{1024*40, 5, false, "weather_task", tskNO_AFFINITY};
    esp_pthread_set_cfg(&cfg);
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
}

static void on_wifi_connected(const char* ip) {
    local_ip_now = ip;
    start_rpc_task();
    start_weather_task();
}

static void start_wifi_task() {
    auto nvs = nvs::open_nvs_handle(NS_NAME_WIFI, NVS_READWRITE);
    bool configed = false;
    nvs->get_item("configed", configed);
    if (configed) {
        char ssid[32];
        char passwd[64];
        nvs->get_string("ssid", ssid, sizeof(ssid));
        nvs->get_string("passwd", passwd, sizeof(passwd));
        wifi_station_init(ssid, passwd, [](const char* ip){
            on_wifi_connected(ip);
        });
    }
    else {
        start_smartconfig_task([](const WiFiInfo& info) {
            ESP_LOGI(TAG, "WiFiInfo:%s %s", info.ssid.c_str(), info.passwd.c_str());
            auto nvs = nvs::open_nvs_handle(NS_NAME_WIFI, NVS_READWRITE);
            nvs->set_string("ssid", info.ssid.c_str());
            nvs->set_string("passwd", info.passwd.c_str());
            nvs->set_item("configed", true);
            nvs->commit();
        }, [](ConnectState state, void *data) {
            ESP_LOGI(TAG, "ConnectState:%d", (int)state);
            if (state == ConnectState::Connected) {
                on_wifi_connected((char*)data);
            }
        });
    }
}

extern "C"
void app_main() {
    oled.begin();

    nvs_init();
    start_wifi_task();

    ScreenConfig c{};
    c.SCREEN_WIDTH = 128;
    c.SCREEN_HEIGHT = 64;
    c.PER_CHAR_WIDTH = 6;
    c.fps = 120;
    c.canvas = &screen;
    start_game(&c);
}
