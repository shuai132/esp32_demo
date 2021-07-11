idf_component_register(SRCS "main.cpp"
        game/game_engine_port_esp32_idf.cpp
        oled/OLED.cpp
        oled/u8g2_esp32_hal.cpp
        modules/chrome_game/game/chrome_game.cpp
        INCLUDE_DIRS ".")

include_directories(modules)

set(MYAPP __idf_main)
add_subdirectory(modules/chrome_game/engine)
target_link_libraries(${MYAPP} PUBLIC game_engine)
target_include_directories(${MYAPP} PUBLIC modules/chrome_game/game)

add_subdirectory(modules/u8g2)
target_link_libraries(${MYAPP} PUBLIC u8g2)

