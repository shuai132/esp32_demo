#pragma once

#include <string>
#include <functional>

struct WiFiInfo {
    std::string ssid;
    std::string passwd;
};

enum class ConnectState {
    Disconnected,
    Connecting,
    Connected,
};

void start_smartconfig_task(std::function<void(WiFiInfo)> updateCb,
                            std::function<void(ConnectState)> connectCb);
