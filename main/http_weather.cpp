#include "http_weather.h"

#include <esp_system.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <unity.h>
#include <nvs_handle.hpp>
#include <utility>
#include "defer.h"

const static char *HTTP_TAG = "WEATHER";
const static char *NS_NAME_WEATHER = "WEATHER";

#define API_KEY_DEFAULT         "S9r6pqlLjzI_cKVZM"
#define API_LOCATION_DEFAULT    "31.315207:121.513113"

static std::string api_key;
static std::string api_location;

void http_weather_init() {
    auto nvs = nvs::open_nvs_handle(NS_NAME_WEATHER, NVS_READWRITE);
    bool configed = false;
    nvs->get_item("configed", configed);
    if (!configed) {
        api_key = API_KEY_DEFAULT;
        api_location = API_LOCATION_DEFAULT;
        nvs->set_string("key", api_key.c_str());
        nvs->set_string("location", api_location.c_str());
        nvs->set_item("configed", true);
        nvs->commit();
    } else {
        char tmp[64];
        nvs->get_string("key", tmp, sizeof(tmp));
        api_key.assign(tmp);
        nvs->get_string("location", tmp, sizeof(tmp));
        api_location.assign(tmp);
    }
}

void http_weather_config_key(std::string key) {
    api_key = std::move(key);
    auto nvs = nvs::open_nvs_handle(NS_NAME_WEATHER, NVS_READWRITE);
    nvs->set_string("key", api_key.c_str());
    nvs->commit();
}

void http_weather_config_location(std::string location) {
    api_location = std::move(location);
    auto nvs = nvs::open_nvs_handle(NS_NAME_WEATHER, NVS_READWRITE);
    nvs->set_string("location", api_location.c_str());
    nvs->commit();
}

std::string http_weather_get() {
    std::string api_url = "http://api.seniverse.com/v3/weather/now.json?key=" + api_key
        + "&location=" + api_location
        + "&language=en&unit=c";
    esp_http_client_config_t config_with_url = {
            .url = api_url.c_str()
    };
    esp_http_client_handle_t client = esp_http_client_init(&config_with_url);
    TEST_ASSERT(client != nullptr);
    defer {
        esp_http_client_cleanup(client);
    };

    // GET Request
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    defer {
        esp_http_client_close(client);
    };
    if (err != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(HTTP_TAG, "HTTP client fetch headers failed");
        } else {
            std::string result;
            result.resize(content_length);
            int data_read = esp_http_client_read_response(client, result.data(), content_length);
            if (data_read >= 0) {
                ESP_LOGI(HTTP_TAG, "HTTP GET Status = %d, content_length = %d",
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));
                return result;
            } else {
                ESP_LOGE(HTTP_TAG, "Failed to read response");
            }
        }
    }
    return "";
}
