#ifndef MY_BT_H
#define MY_BT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_bt.h"

#include "esp_hidd_prf_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "driver/gpio.h"
#include "hid_dev.h"

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

typedef struct {
    void (*on_device_connected)(esp_bd_addr_t bd_addr, esp_ble_addr_type_t addr_type, bool known_device);
} bt_api_callbacks_t;

bool bt_is_connected();
void bt_start_advertising();
void bt_stop_advertising();
void bt_disconnect(esp_bd_addr_t connected_device_address);
bool bt_direct_advertizing(esp_bd_addr_t addr, esp_ble_addr_type_t addr_type);

void bt_hid_send_keyboard_string_sequence(const char *s);

void init_bluetooth(bt_api_callbacks_t *callbacks);
//void stop_advertising();

#endif

