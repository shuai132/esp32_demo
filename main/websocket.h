#pragma once

#include <string>
#include <functional>
#include <esp_websocket_client.h>
#include "noncopyable.hpp"

class WebsocketClient : noncopyable {
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

    std::function<void(std::string package)> onReceivedData;
    std::function<void(ConnectState)> onConnectState;

    std::string package;
private:
    esp_websocket_client_handle_t client;
};
