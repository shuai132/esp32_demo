#pragma once

#include <functional>

using on_get_ip_t = std::function<void(const char* ip)>;

void wifi_station_init(const char* ssid, const char* passwd, on_get_ip_t handle = nullptr, int retry_num_max = 20);
bool wait_for_connect();
