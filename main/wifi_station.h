#pragma once

#include <functional>

using on_get_ip_t = std::function<void(const char* ip)>;

void wifi_init();
void wifi_station_init(const char* ssid, const char* passwd, on_get_ip_t on_get_ip_handle = nullptr, std::function<void()> on_connect_failed_handle = nullptr, int retry_num_max = 20);
bool wifi_station_wait_for_connect();
void wifi_station_release();
