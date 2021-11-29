#include <thread>
#include <sstream>
#include <utility>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_pthread.h>
#include <driver/gpio.h>
#include "game_engine.hpp"
#include "chrome_game.h"
#include "game/game_engine_port_esp32_idf.h"
#include "wifi_station.h"
#include "http_weather.h"
#include "NetChannel.h"
#include "RpcCore.hpp"
#include "ArduinoJson.hpp"
#include "asio.hpp"
#include "utils.h"
#include "smartconfig.h"
#include "esp_init.h"
#include "udp_multicast.hpp"
#include "udp_server.hpp"

const static char* TAG = "MAIN";
const static char* NS_NAME_WIFI = "wifi";
#ifdef CONFIG_IDF_TARGET_ESP32
static Screen screen;
#else
struct ScreenFake : public ge::Canvas {
    void onClear() override {}
    void onDraw() override{ sleep(1); }
    void drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color) override {}
    size_t drawBuffer(uint16_t x, uint16_t y, const char *buffer, size_t len) override{ return 0; }
    std::function<void()> onBeforeDraw;
};
static ScreenFake screen;
#endif
static std::string local_ip_now;
static std::atomic_bool screen_show_rpc{false};
static std::atomic_bool screen_show_weather{false};
static ge::Director* game;
static asio::io_context main_context;

static void start_game_task() {
    std::thread([]{
        ScreenConfig c{};
        c.SCREEN_WIDTH = 128;
        c.SCREEN_HEIGHT = 64;
        c.PER_CHAR_WIDTH = 6;
        c.canvas = &screen;
        game = static_cast<ge::Director *>(chrome_game_init(&c));
        game->start(120);
    }).detach();
}

static void start_rpc_task() {
    static std::shared_ptr<RpcCore::Rpc> rpc;
    static NetChannel client(&main_context);

    using namespace RpcCore;
    using RpcCore::String;
    auto connection = std::make_shared<Connection>();
    connection->sendPackageImpl = [](std::string package) {
        client.sendData(std::move(package));
    };
    client.onData = [connection](std::string data) {
        connection->onRecvPackage(std::move(data));
    };
    client.onOpen = [] {
        screen_show_rpc = true;
        game->pause();
        screen.onClear();
    };
    client.onClose = []{
        screen_show_rpc = false;
        screen.onClear();
        game->resume();
    };

    rpc = Rpc::create(connection);
    rpc->setTimer([&](uint32_t ms, Rpc::TimeoutCb cb) {
        utils::steady_timer(&main_context, std::move(cb), ms);
    });
    rpc->subscribe<String>("config_api", [](String key) {
        http_weather_config_key(std::move(key));
    });
    rpc->subscribe<String>("config_location", [](String location) {
        http_weather_config_location(std::move(location));
    });

    static asio_udp::udp_server udp_server(main_context, 3001, 1024);
    udp_server.onData = [](uint8_t* data, size_t size, const asio_udp::udp::endpoint& from) {
        if (!screen_show_rpc) return;
        if (size < 1024) return;
        screen.onClear();
        screen.drawBitmap(0, 0, data, 128, 64, 1);
        screen.onDraw();
    };

    static udp_multicast::receiver receiver(main_context, [](const std::string& name, const std::string& message) {
        if (name != "opencv_oled") return;
        static std::string uri;
        if (client.isOpen() && uri == message) return;
        ESP_LOGI(TAG, "found server: %s", message.c_str());
        uri = message;
        client.close();
        auto pos = uri.find(':');
        auto ip = uri.substr(0, pos);
        auto port = uri.substr(pos+1);
        client.start(ip, port);
    });
}

static void start_weather_task() {
    static std::string temperature;
    static std::string weather;
    static std::string update_time;

    http_weather_init();

    screen.onBeforeDraw = [] {
        if (screen_show_rpc) return;
        if (!screen_show_weather) return;
        screen.drawString(0, 8*0, local_ip_now.c_str());
        screen.drawString(0, 8*1, temperature.c_str());
        screen.drawString(0, 8*2, weather.c_str());
        screen.drawString(0, 8*3, update_time.c_str());
    };

    esp_pthread_cfg_t cfg{1024*40, 5, false, "weather_task", tskNO_AFFINITY};
    esp_pthread_set_cfg(&cfg);
    std::thread([]{
        for(;;) {
            if (!screen_show_weather | screen_show_rpc) {
                sleep(1);
                continue;
            }
            std::string http_result = http_weather_get();
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
            update_time = update_time.substr(0, update_time.find('+'));

            sleep(60);
        }
    }).detach();
}

static void on_wifi_connected(const char* ip) {
    local_ip_now = ip;
    screen_show_weather = true;
}

static void start_wifi_task() {
    static auto start_smartconfig = []{
        screen_show_weather = false;
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
    };

    static auto config_button_isr = []{
        static xQueueHandle gpio_evt_queue = xQueueCreate(8, 1);
        gpio_install_isr_service(0);
        gpio_set_intr_type(GPIO_NUM_0, GPIO_INTR_ANYEDGE);
        gpio_isr_handler_add(GPIO_NUM_0, [](void*) IRAM_ATTR {
            uint8_t value = gpio_get_level(GPIO_NUM_0);
            xQueueSendFromISR(gpio_evt_queue, &value, nullptr);
            }, nullptr);

        std::thread([]{
            static unsigned long pushTime;
            for(;;) {
                uint8_t value;
                if(!xQueueReceive(gpio_evt_queue, &value, portMAX_DELAY)) continue;
                ESP_LOGI(TAG, "button: %d", value);
                if(value == 0) {
                    pushTime = ge::nowMs();
                } else {
                    if (ge::nowMs() - pushTime >= 3000) {
                        ESP_LOGI(TAG, "start smartconfig");
                        start_smartconfig();
                    }
                }
            }
        }).detach();
    };

    config_button_isr();

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
        }, []{
            start_smartconfig();
        });
    }
    else {
        start_smartconfig();
    }
}

extern "C"
void app_main() {
    esp_init();
//    initButton();
//    oled.begin();
    start_game_task();
    start_wifi_task();
    start_rpc_task();
    start_weather_task();
    asio::io_context::work work(main_context);
    main_context.run();
}
