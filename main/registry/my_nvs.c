#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "my_nvs.h"

static const char *TAG = "MY_NVS";

void my_nvs_open(nvs_handle_t *my_handle, const char *namespace) {
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}

void my_nvs_commit_and_close(nvs_handle_t my_handle) {
    esp_err_t err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) committing NVS handle!\n", esp_err_to_name(err));
    }
    nvs_close(my_handle);
}

void my_nvs_erase_key(nvs_handle_t my_handle, const char *key) {
    esp_err_t err = nvs_erase_key(my_handle, key);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) erasing key!\n", esp_err_to_name(err));
    }
}

void my_nvs_set_i16(nvs_handle_t my_handle, const char *key, int16_t value) {
    esp_err_t err = nvs_set_i16(my_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting %s: %s\n", key, esp_err_to_name(err));
    }
}

int16_t my_nvs_get_i16(nvs_handle_t my_handle, const char *key, const int16_t default_value) {
    int16_t value;
    esp_err_t err = nvs_get_i16(my_handle, key, &value);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No %s stored yet", key);
        value = default_value;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading %s from NVS: %s", key, esp_err_to_name(err));
        value = default_value;
    }
    return value;
}

void my_nvs_save_password_dialog_settings(int length, bool use_numbers, bool use_symbols_set1, bool use_symbols_set2) {
    nvs_handle_t my_handle;
    my_nvs_open(&my_handle, "password_dialog");
    my_nvs_set_i16(my_handle, "length", length);
    my_nvs_set_i16(my_handle, "use_numbers", use_numbers);
    my_nvs_set_i16(my_handle, "use_symbols1", use_symbols_set1);
    my_nvs_set_i16(my_handle, "use_symbols2", use_symbols_set2);
    my_nvs_commit_and_close(my_handle);
}

void my_nvs_load_password_dialog_settings(int *length, bool *use_numbers, bool *use_symbols_set1, bool *use_symbols_set2) {
    nvs_handle_t my_handle;
    my_nvs_open(&my_handle, "password_dialog");
    *length = my_nvs_get_i16(my_handle, "length", 20);
    *use_numbers = my_nvs_get_i16(my_handle, "use_numbers", 1);
    *use_symbols_set1 = my_nvs_get_i16(my_handle, "use_symbols1", 1);
    *use_symbols_set2 = my_nvs_get_i16(my_handle, "use_symbols2", 0);
    nvs_close(my_handle);
}


