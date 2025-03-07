#include "power_control.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static void check_power_button_task(void *arg) {
    while (1) {
        if (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
            vTaskDelay(pdMS_TO_TICKS(2000)); 
            if (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
                gpio_set_level(SYS_EN_GPIO, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void init_gpio_and_power_task() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SYS_EN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(SYS_EN_GPIO, 1);

    io_conf.pin_bit_mask = (1ULL << PWR_BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    xTaskCreate(check_power_button_task, "check_power_button_task", 2048, NULL, 5, NULL);
}
