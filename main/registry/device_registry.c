#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "item_registry.h"
#include "my_nvs.h"

static const char *TAG = "DEV_REG";

static device_entry_t* devices = NULL;
static int16_t count;

#define MAX_DEVICES (16)
#define NVS_STORAGE_NAME "device_storage"
#define NVS_KEY_COUNT "device_count"
#define NVS_KEY_FORMAT "dev_%d"

device_entry_t current_device;

void device_registry_load() {
    esp_err_t err;
    nvs_handle_t my_handle;
    char key[16];

    uint16_t id = 0;
    int loaded_count = 0;

    ESP_LOGI(TAG, "Loading devices");
    if (devices == NULL) {
        devices = (device_entry_t *)malloc(MAX_DEVICES * sizeof(device_entry_t));
    }

    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    count = my_nvs_get_i16(my_handle, NVS_KEY_COUNT);

    for (int i = 0; i < count; i++) {
        do {
            sprintf(key, NVS_KEY_FORMAT, ++id);
        } while (nvs_find_key(my_handle, key, NULL) == ESP_ERR_NOT_FOUND);

        size_t required_size;
        err = nvs_get_blob(my_handle, key, NULL, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error reading device data from NVS: %s", esp_err_to_name(err));
            continue;
        }
        if (required_size != sizeof(device_entry_t)) {
            ESP_LOGE(TAG, "Invalid device data size: %d", required_size);
            continue;
        }
        err = nvs_get_blob(my_handle, key, &devices[i], &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error reading device data from NVS: %s", esp_err_to_name(err));
            continue;
        }
        loaded_count++;
    }

    nvs_close(my_handle);
    if (count != loaded_count) {
        ESP_LOGE(TAG, "Loaded %d devices of %d", loaded_count, count);
    } else {
        ESP_LOGI(TAG, "Loaded %d devices", loaded_count);
    }
}

static uint16_t get_next_id() {
    uint16_t id = 1;
    for (int i = 0; i < count; i++) {
        if (devices[i].id == id) {
            id++;
            i = -1;
        }
    }
    return id;
}

void device_registry_add_new_device(device_entry_t* device) {
    esp_err_t err;
    nvs_handle_t my_handle;
    char key[16];
    uint16_t id = get_next_id();
    sprintf(key, NVS_KEY_FORMAT, id);

    if (count >= MAX_DEVICES) {
        ESP_LOGE(TAG, "Device registry full");
        return;
    }
    device->id = id;
    devices[count] = *device;
    count++;
    
    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    my_nvs_set_i16(my_handle, NVS_KEY_COUNT, count);
    err = nvs_set_blob(my_handle, key, device, sizeof(device_entry_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) setting device data!\n", esp_err_to_name(err));
    }
    my_nvs_commit_and_close(my_handle);
    ESP_LOGI(TAG, "New Device Saved, name=%s, id=%d, total count %d", device->name, id, count);
}

void device_registry_remove_device(int i) {
    nvs_handle_t my_handle;
    char key[16];
    uint16_t id = devices[i].id;
    sprintf(key, NVS_KEY_FORMAT, id);

    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    count--;
    my_nvs_set_i16(my_handle, NVS_KEY_COUNT, count);
    my_nvs_erase_key(my_handle, key);
    my_nvs_commit_and_close(my_handle);

    for (int j = i; j < count; j++) {
        devices[j] = devices[j + 1];
    }
    ESP_LOGI(TAG, "Device Removed, total count %d", count);
}

device_entry_t* device_registry_get_by_index(int i) {
    return i < count ? &devices[i] : NULL;
}

int device_registry_get_index_by_name(const char* name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(devices[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

device_entry_t* device_registry_get_by_address(const esp_bd_addr_t addr) {
    for (int i = 0; i < count; i++) {
        if (memcmp(devices[i].addr, addr, sizeof(esp_bd_addr_t)) == 0) {
            return &devices[i];
        }
    }
    return NULL;
}

static int get_stored_count() {
    return count;
}

static char* get_name(int i) {
    return devices[i].name;
}

registry_api_t device_registry_common = {
    .get_name = get_name,
    .get_count = get_stored_count
};
