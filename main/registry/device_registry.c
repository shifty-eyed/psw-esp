#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "item_registry.h"

static const char *TAG = "DEVICE_REGISTRY";

static device_entry_t* devices = NULL;
static int16_t count;

#define MAX_DEVICES (16)
#define NVS_STORAGE_NAME "device_storage"
#define NVS_KEY_COUNT "device_count"
#define NVS_KEY_FORMAT "dev_%d"


static void read_device_count(nvs_handle_t my_handle) {
    esp_err_t err;
    err = nvs_get_i16(my_handle, NVS_KEY_COUNT, &count);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No device count stored yet");
        count = 0;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading device count from NVS: %s", esp_err_to_name(err));
        count = 0;
    }
}

static void read_device_data(nvs_handle_t my_handle) {
    esp_err_t err;
    char key[16];

    for (int i = 0; i < count; i++) {
        sprintf(key, NVS_KEY_FORMAT, i);
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
    }
}

static void load() {
    ESP_LOGI(TAG, "Loading devices");
    if (devices == NULL) {
        devices = (device_entry_t *)malloc(MAX_DEVICES * sizeof(device_entry_t));
    }

    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open(NVS_STORAGE_NAME, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }
    read_device_count(my_handle);
    read_device_data(my_handle);

    nvs_close(my_handle);
    ESP_LOGI(TAG, "Loaded %d devices", count);
}

static void save() {
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open(NVS_STORAGE_NAME, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_i16(my_handle, NVS_KEY_COUNT, count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) setting count!\n", esp_err_to_name(err));
    }

    char key[16];
    for (int i = 0; i < count; i++) {
        sprintf(key, NVS_KEY_FORMAT, i);
        err = nvs_set_blob(my_handle, key, &devices[i], sizeof(device_entry_t));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) setting device data!\n", esp_err_to_name(err));
        }
    }

    nvs_commit(my_handle);
    nvs_close(my_handle);
    ESP_LOGI(TAG, "Saved %d devices", count);
}

void device_registry_add_new_device(const device_entry_t* device) {
    if (count >= MAX_DEVICES) {
        ESP_LOGE(TAG, "Device registry full");
        return;
    }
    devices[count] = *device;
    count++;
    ESP_LOGI(TAG, "Device Added, total count %d", count);
    save();
}

static void device_remove(int i) {
    count--;
    for (int j = i; j < count; j++) {
        devices[j] = devices[j + 1];
    }
    ESP_LOGI(TAG, "Device Removed, total count %d", count);
    save();
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

static void destroy() {
    free(devices);
}

registry_api_t device_registry_common = {
    .remove = device_remove,
    .get_name = get_name,
    .get_count = get_stored_count,
    .load = load,
    .destroy = destroy
};
