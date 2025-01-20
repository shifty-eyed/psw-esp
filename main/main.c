#include "lcd_touch.h"
#include "item_registry.h"
#include "ui.h"
#include "my_bt.h"

static const char *TAG = "MAIN";


static void on_device_paired(esp_bd_addr_t* bd_addr, esp_link_key* key) {
    ESP_LOGI(TAG, "Device paired");
    //device_registry.load();
    lvgl_port_lock(-1);
    ui_on_device_paired();
    lvgl_port_unlock();
}

static void add_new_device() {
    ESP_LOGI(TAG, "add_new_device()");
    bt_start_advertising();
}

static ui_command_callbacks_t ui_commands = {
    .add_new_device = add_new_device
};

static bt_callbacks_t bt_api_callbacks = {
    .on_device_paired = on_device_paired
};

void app_main(void) {
    init_lcd_and_touch();
    set_lcd_brightness(50);

    device_registry.load();
    password_registry.load();

    lvgl_port_lock(-1);


    init_ui(&ui_commands);
    lvgl_port_unlock();

    init_bluetooth(&bt_api_callbacks);

}
