#pragma once

#include <string>
#include <functional>
#include <esp_websocket_client.h>

class WebsocketClient {
public:
    enum class ConnectState {
        Connected,
        Disconnected,
        Closed,
    };

public:
    WebsocketClient();

    ~WebsocketClient();

    void start(const std::string &uri);

    void close();

    void send(void *data, size_t len) const;

    std::function<void(void *data, size_t len)> onReceivedData;
    std::function<void(ConnectState)> onConnectState;

private:
    esp_websocket_client_handle_t client;
};
