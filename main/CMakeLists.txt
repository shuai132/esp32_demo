idf_component_register(SRCS "main.cpp"
        game/game_engine_port_esp32_idf.cpp
        oled/OLED.cpp
        oled/Adafruit_GFX.cpp
        oled/Adafruit_SSD1306.cpp
        oled/glcdfont.c
        modules/chrome_game/game/chrome_game.cpp
        INCLUDE_DIRS ".")

include_directories(modules)

set(MYAPP __idf_main)
add_subdirectory(modules/chrome_game/engine)
target_link_libraries(${MYAPP} PUBLIC game_engine)
target_include_directories(${MYAPP} PUBLIC modules/chrome_game/game)
target_compile_options(${MYAPP} PRIVATE -Wno-missing-field-initializers)
