#include <esp_netif.h>
#include <driver/gpio.h>
#include "asio.hpp"
#include "TcpServer.hpp"

#define LED_R GPIO_NUM_3
#define LED_G GPIO_NUM_4
#define LED_B GPIO_NUM_5

extern "C" void wifi_init_sta(void);

static void loop() {
    wifi_init_sta();

    asio::io_context io_context;
    TcpServer tcp_server(io_context, 3333);
    io_context.post([]{
        printf("hello\n");
    });
    std::thread send([&]{
        uint32_t count = 0;
        for(;;) {
            sleep(1);
            char msg[8]{};
            sprintf(msg, "%u\n", count++);
            tcp_server.writeAll(reinterpret_cast<const uint8_t*>(msg), strlen(msg));
        }
    });
    io_context.run();
}

extern "C"
void app_main() {
    printf("hello\n");

    gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << LED_R | 1ULL << LED_G | 1ULL << LED_B,
            .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    std::thread dd([]{
        int count = 0;
        for(;;) {
            printf("thread: %d\n", count++);
            gpio_set_level(static_cast<gpio_num_t>(LED_R + count % 3), 1);
            std::this_thread::sleep_for(std::chrono::milliseconds (100));
            gpio_set_level(static_cast<gpio_num_t>(LED_R + count % 3), 0);
            std::this_thread::sleep_for(std::chrono::milliseconds (100));
        }
    });
    loop();
    dd.join();
}
