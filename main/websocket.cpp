#include "websocket.h"
#include "freertos/FreeRTOS.h"
#include "esp_websocket_client.h"
#include "esp_log.h"

static const char* TAG = "WEBSOCKET";

static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    auto client = static_cast<WebsocketClient*>(handler_args);
    auto* data = (esp_websocket_event_data_t*) event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            if (client->onConnectState) {
                client->onConnectState(WebsocketClient::ConnectState::Disconnected);
            }
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGI(TAG, "Received opcode=%d, len=%d", data->op_code, data->data_len);
            if (data->op_code == 0x08 && data->data_len == 2) {
                ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
                if (client->onConnectState) {
                    client->onConnectState(WebsocketClient::ConnectState::Closed);
                }
            } else {
                if (data->payload_offset == 0) {
                    client->package.resize(0);
                    client->package.reserve(data->payload_len);
                }
                client->package.append(data->data_ptr, data->data_len);
                if (client->onReceivedData && data->data_len > 0 && client->package.length() == data->payload_len) {
                    client->onReceivedData(client->package);
                }
            }
            ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d", data->payload_len,
                     data->data_len, data->payload_offset);
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}

WebsocketClient::WebsocketClient() {
//    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
//    esp_log_level_set("TRANSPORT_WS", ESP_LOG_DEBUG);
//    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);
}

WebsocketClient::~WebsocketClient() {
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(client);
}

void WebsocketClient::start(const std::string& uri) {
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = uri.c_str();
    websocket_cfg.buffer_size = 1024 * 2;
    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);
    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, this);
    esp_websocket_client_start(client);
}

void WebsocketClient::close() {
    esp_websocket_client_close(client, portMAX_DELAY);
}

void WebsocketClient::send(void* data, size_t len) const {
    esp_websocket_client_send_text(client, (const char*) data, (int) len, portMAX_DELAY);
}
