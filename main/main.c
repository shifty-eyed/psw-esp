#include "nvs_flash.h"

#include "system/lcd_touch.h"
#include "registry/item_registry.h"
#include "ui/ui.h"
#include "bt/my_bt.h"

static const char *TAG = "MAIN";

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

/*
TODO:

- make ui button to type password
- make ui for new password generation
- make ui for password change

 */

void app_main(void) {
    esp_err_t ret;
    init_lcd_and_touch();
    set_lcd_brightness(50);


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
    lvgl_port_unlock();

    init_bluetooth(&bt_api_callbacks);
    esp_log_level_set(HID_LE_PRF_TAG, ESP_LOG_DEBUG);

}
