#include "nvs_flash.h"

#include "esp_sleep.h"
#include "system/lcd_touch.h"
#include "system/power_control.h"
#include "registry/item_registry.h"
#include "ui/ui.h"
#include "bt/my_bt.h"

static const char *TAG = "MAIN";

#define SYS_EN_GPIO  GPIO_NUM_35

/* Bluetooth module callbacks */
static void on_device_connected(esp_bd_addr_t bd_addr, esp_ble_addr_type_t addr_type, bool known_device) {
    device_entry_t *device = device_registry_get_by_address(bd_addr);
    memcpy(current_device.addr, bd_addr, sizeof(esp_bd_addr_t));
    current_device.addr_type = addr_type;

    if (known_device) {
        if (device == NULL) {
            ESP_LOGE(TAG, "Known device but not found in registry");
            return;
        }
        ESP_LOGI(TAG, "Known Device connected");
        int index = device_registry_get_index_by_name(device->name);

        lvgl_port_lock(-1);
        ui_on_known_device_connected(index);
        lvgl_port_unlock();
    } else {
        if (device != NULL) {
            ESP_LOGE(TAG, "New Device connected but already in registry");
            //maybe show toaster
            return;
        }
        ESP_LOGI(TAG, "New Device Paired");
        lvgl_port_lock(-1);
        ui_on_new_device_paired();
        lvgl_port_unlock();
    }
}

static bt_api_callbacks_t bt_api_callbacks = {
    .on_device_connected = on_device_connected
};

static touch_callbacks_t touch_callbacks = {
    .on_press = ui_on_touch_pressed,
    .on_release = NULL
};

void app_main(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "esp_sleep_is_valid_wakeup_gpio: %d", esp_sleep_is_valid_wakeup_gpio(PWR_BUTTON_GPIO));
    ESP_LOGI(TAG, "Woke up from: %d", esp_sleep_get_wakeup_cause());
    init_gpio_and_power_task();
    init_lcd_and_touch(&touch_callbacks);
    //set_lcd_brightness(40);


    // Initialize NVS.
    //ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    device_registry_load();
    password_registry_load();

    lvgl_port_lock(-1);
    init_ui();
    //init_touch_test_ui();
    lvgl_port_unlock();

    init_bluetooth(&bt_api_callbacks);
    esp_log_level_set(HID_LE_PRF_TAG, ESP_LOG_DEBUG);

}