#include "power_control.h"
#include "freertos/FreeRTOS.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"

#include "system/lcd_touch.h"

#define TAG "POWER_CONTROL"

static void check_power_button_task(void *arg) {
    while (1) {
        if (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
            while (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            ESP_LOGI(TAG, "Entering (deep sleep)...");

            rtc_gpio_init(PWR_BUTTON_GPIO);
            rtc_gpio_pullup_en(PWR_BUTTON_GPIO);
            rtc_gpio_pulldown_dis(PWR_BUTTON_GPIO);

            esp_sleep_enable_ext0_wakeup(PWR_BUTTON_GPIO, 0);
            lcd_panel_on(false);
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_deep_sleep_start();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void init_gpio_and_power_task() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    xTaskCreate(check_power_button_task, "check_power_button_task", 2048, NULL, 5, NULL);
}
