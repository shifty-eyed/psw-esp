#include "nvs_flash.h"

#include "lcd_touch.h"
#include "item_registry.h"
#include "ui.h"
#include "my_bt.h"

static const char *TAG = "MAIN";

/* User actions */
static void action_add_new_device() {
    ESP_LOGI(TAG, "add_new_device()");
    bt_start_advertising();
}

static void action_connect_to_device(int i) {
    ESP_LOGI(TAG, "action_connect_to_device (%d)", i);
    device_entry_t *device = device_get(i);
    bt_direct_connect(device->addr, device->addr_type);
}

static void action_disconnect() {
    ESP_LOGI(TAG, "disconnect()");
    bt_disconnect();
}

static ui_api_callbacks_t user_action_callbacks = {
    .add_new_device = action_add_new_device,
    .connect_to_device = action_connect_to_device,
    .disconnect = action_disconnect
};


/* Bluetooth module callbacks */
static void on_device_connected(esp_bd_addr_t* bd_addr, esp_ble_addr_type_t* addr_type, bool known_device) {
    if (known_device) {
        ESP_LOGI(TAG, "Device connected");
        return;
    } else {
        ESP_LOGI(TAG, "Device paired");

        //TODO: ask for device name in the ui

        device_entry_t new_device;
        memcpy(new_device.addr, bd_addr, sizeof(esp_bd_addr_t));
        new_device.addr_type = *addr_type;
        strcpy(new_device.name, "Computer");
        device_add(&new_device);

        lvgl_port_lock(-1);
        ui_on_device_connected(known_device);
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
    //ESP_ERROR_CHECK(nvs_flash_erase());
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
