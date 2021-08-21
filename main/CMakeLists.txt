idf_component_register(SRCS "main.cpp"
        game/game_engine_port_esp32_idf.cpp
        oled/OLED.cpp
        wifi_station.c
        oled/Adafruit_GFX.cpp
        oled/Adafruit_SSD1306.cpp
        oled/glcdfont.c
        modules/chrome_game/game/chrome_game.cpp
        http_weather.cpp
        websocket.cpp
        INCLUDE_DIRS ".")

include_directories(modules base)

set(MYAPP __idf_main)
add_subdirectory(modules/chrome_game/engine)
target_link_libraries(${MYAPP} PUBLIC game_engine)
target_include_directories(${MYAPP} PUBLIC modules/chrome_game/game)
target_compile_options(${MYAPP} PRIVATE -Wno-missing-field-initializers)

add_subdirectory(modules/RpcCore)
target_link_libraries(${MYAPP} PUBLIC RpcCore)
target_compile_definitions(${MYAPP} PRIVATE -DLOG_FOR_MCU)
