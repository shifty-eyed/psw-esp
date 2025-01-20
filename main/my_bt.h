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
#include "nvs_flash.h"
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
    void (*on_device_paired)(esp_bd_addr_t* bd_addr, esp_link_key* key);
} bt_callbacks_t;

void bt_start_advertising();
void init_bluetooth(bt_callbacks_t *callbacks);
//void stop_advertising();

#endif