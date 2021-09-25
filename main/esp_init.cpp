#include "esp_init.h"
#include <nvs_flash.h>
#include <esp_event_loop.h>
#include "oled/OLED.h"
#include "wifi_station.h"

void esp_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    oled.begin();

    wifi_init();
}
