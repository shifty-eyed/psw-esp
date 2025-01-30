#include "nvs_flash.h"

#include "system/lcd_touch.h"
#include "registry/item_registry.h"
#include "ui/ui.h"
#include "bt/my_bt.h"

static const char *TAG = "MAIN";

static device_entry_t current_device;

/* User actions */
static void action_start_pairing() {
    ESP_LOGI(TAG, "pairing_initiated()");
    bt_start_advertising();
}

static void action_connect_to_device(int i) {
    ESP_LOGI(TAG, "action_connect_to_device (%d)", i);
    device_entry_t *device = device_registry_get_by_index(i);
    bt_direct_advertizing(device->addr, device->addr_type);
}

static void action_cancel_pairing() {
    ESP_LOGI(TAG, "pair_device_dialog_cancel()");
    bt_stop_advertising();
}

static void action_disconnect() {
    ESP_LOGI(TAG, "disconnect()");
    bt_disconnect(current_device.addr);
}

static void action_save_new_device(const char *name) {
    ESP_LOGI(TAG, "save_new_device(%s)", name);
    strcpy(current_device.name, name);
    
    device_registry_add_new_device(&current_device);
    int index = device_registry_get_index_by_name(name);
    lvgl_port_lock(-1);
    ui_on_new_device_saved(index);
    lvgl_port_unlock();
}

static ui_api_callbacks_t user_action_callbacks = {
    .pairing_initiated = action_start_pairing,
    .cancel_pairing = action_cancel_pairing,
    .connect_to_device = action_connect_to_device,
    .disconnect = action_disconnect,
    .save_new_device = action_save_new_device
};


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

void app_main(void) {
    esp_err_t ret;
    init_lcd_and_touch();
    set_lcd_brightness(50);


    // Initialize NVS.
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    device_registry_common.load();
    password_registry_common.load();

    lvgl_port_lock(-1);
    init_ui(&user_action_callbacks);
    lvgl_port_unlock();

    init_bluetooth(&bt_api_callbacks);

}
