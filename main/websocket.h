#pragma once

#include <string>
#include <functional>
#include "noncopyable.hpp"

class esp_websocket_client;
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
    esp_websocket_client* client;
};
