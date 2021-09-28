#pragma once

#include <string>

void http_weather_init();

void http_weather_config_key(std::string key);

void http_weather_config_location(std::string location);

std::string http_weather_get();
