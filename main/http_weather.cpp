#include "http_weather.h"

#include <cstdlib>
#include <esp_system.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <unity.h>
#include "defer.h"

const static char *HTTP_TAG = "WEATHER";

std::string http_get_weather() {
    esp_http_client_config_t config_with_url = {
            .url = "http://api.seniverse.com/v3/weather/now.json?key=S9r6pqlLjzI_cKVZM&location=shanghai&language=en&unit=c"
    };
    esp_http_client_handle_t client = esp_http_client_init(&config_with_url);
    TEST_ASSERT(client != nullptr);
    defer [&]{
        esp_http_client_cleanup(client);
    };

    // GET Request
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    defer [&]{
        esp_http_client_close(client);
    };
    if (err != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(HTTP_TAG, "HTTP client fetch headers failed");
        } else {
            char output_buffer[content_length];
            int data_read = esp_http_client_read_response(client, output_buffer, content_length);
            if (data_read >= 0) {
                ESP_LOGI(HTTP_TAG, "HTTP GET Status = %d, content_length = %d",
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));
                return output_buffer;
            } else {
                ESP_LOGE(HTTP_TAG, "Failed to read response");
            }
        }
    }
    return "";
}
