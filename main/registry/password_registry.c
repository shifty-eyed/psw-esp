#include "esp_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "item_registry.h"
#include "my_nvs.h"

static const char *TAG = "PSW_REG";

#define MAX_ENTRIES (128)
#define NVS_STORAGE_NAME "psw_storage"
#define NVS_KEY_COUNT "psw_count"
#define NVS_KEY_FORMAT "psw_%d"

static const char* GEN_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const char* GEN_NUMBERS = "0123456789";
static const char* GEN_SYMBOLS_SET1 = "!@#$%*_+-=";
static const char* GEN_SYMBOLS_SET2 = "?&()^[]{};:,.<>~|/\\";

static password_entry_t* entries = NULL;
static int16_t count;

void password_registry_generate_password(char* result, int length, bool use_numbers, bool use_symbols_set1, bool use_symbols_set2) {
    char* charset = (char*)malloc(sizeof(char) * (strlen(GEN_LETTERS) + strlen(GEN_SYMBOLS_SET1) + strlen(GEN_SYMBOLS_SET2) + strlen(GEN_NUMBERS) + 1));
    strcpy(charset, GEN_LETTERS);
    if (use_numbers) {
        strcat(charset, GEN_NUMBERS);
    }
    if (use_symbols_set1) {
        strcat(charset, GEN_SYMBOLS_SET1);
    }
    if (use_symbols_set2) {
        strcat(charset, GEN_SYMBOLS_SET2);
    }
    int charset_len = strlen(charset);
    for (int i = 0; i < length; i++) {
        result[i] = charset[esp_random() % charset_len];
    }
    result[length] = '\0';
    free(charset);
}

int password_registry_get_index_by_id(uint16_t id) {
    for (int i = 0; i < count; i++) {
        if (entries[i].id == id) {
            return i;
        }
    }
    return -1;
}

void password_registry_load() {
    esp_err_t err;
    nvs_handle_t my_handle;
    char key[16];

    uint16_t id = 0;
    int loaded_count = 0;

    ESP_LOGI(TAG, "Loading passwords");
    if (entries == NULL) {
        entries = (password_entry_t *)malloc(MAX_ENTRIES * sizeof(password_entry_t));
    }
    if (entries == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for password entries");
        return;
    }
    ESP_LOGI(TAG, "Allocated memory for passwords, count: %d, entry size:%d, total memory: %d bytes",
        MAX_ENTRIES, sizeof(password_entry_t), MAX_ENTRIES * sizeof(password_entry_t));


    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    count = my_nvs_get_i16(my_handle, NVS_KEY_COUNT);

    for (int i = 0; i < count; i++) {
        do {
            sprintf(key, NVS_KEY_FORMAT, ++id);
        } while (nvs_find_key(my_handle, key, NULL) == ESP_ERR_NOT_FOUND);
        
        size_t required_size;
        err = nvs_get_blob(my_handle, key, NULL, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error reading password data from NVS: %s", esp_err_to_name(err));
            continue;
        }
        if (required_size != sizeof(password_entry_t)) {
            ESP_LOGE(TAG, "Invalid password data size: %d, required: %d", required_size, sizeof(password_entry_t));
            my_nvs_erase_key(my_handle, key);
            nvs_commit(my_handle);
            continue;
        }
        err = nvs_get_blob(my_handle, key, &entries[i], &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error reading password data from NVS: %s", esp_err_to_name(err));
            continue;
        }
        loaded_count++;
    }

    nvs_close(my_handle);
    if (count != loaded_count) {
        ESP_LOGE(TAG, "Loaded %d passwords of %d", loaded_count, count);
        count = loaded_count;
    } else {
        ESP_LOGI(TAG, "Loaded %d passwords", loaded_count);
    }
}

static uint16_t get_next_id() {
    uint16_t id = 1;
    for (int i = 0; i < count; i++) {
        if (entries[i].id == id) {
            id++;
            i = -1;
        }
    }
    return id;
}

void password_registry_add_new_password(password_entry_t* entry) {
    esp_err_t err;
    nvs_handle_t my_handle;
    char key[16];
    uint16_t id = get_next_id();
    sprintf(key, NVS_KEY_FORMAT, id);
    ESP_LOGW(TAG, "Saving psw with key: %s", key);

    if (count >= MAX_ENTRIES) {
        ESP_LOGE(TAG, "Password registry full, max: %d", MAX_ENTRIES);
        return;
    }
    entry->id = id;
    entries[count] = *entry;
    count++;
    
    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    my_nvs_set_i16(my_handle, NVS_KEY_COUNT, count);
    err = nvs_set_blob(my_handle, key, entry, sizeof(password_entry_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) saving password data!\n", esp_err_to_name(err));
    }
    my_nvs_commit_and_close(my_handle);
    ESP_LOGI(TAG, "New Passowrd Saved, name=%s, id=%d, total count %d", entry->name, id, count);
}

void password_registry_update_password(password_entry_t* entry) {
    esp_err_t err;
    nvs_handle_t my_handle;
    char key[16];
    sprintf(key, NVS_KEY_FORMAT, entry->id);

    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    err = nvs_set_blob(my_handle, key, entry, sizeof(password_entry_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) updating password data!\n", esp_err_to_name(err));
    }
    my_nvs_commit_and_close(my_handle);
    memcpy(&entries[password_registry_get_index_by_id(entry->id)], entry, sizeof(password_entry_t));
    ESP_LOGI(TAG, "Password Updated, name=%s, id=%d", entry->name, entry->id);
}

void password_registry_remove_password(uint16_t id) {
    nvs_handle_t my_handle;
    char key[16];
    sprintf(key, NVS_KEY_FORMAT, id);

    my_nvs_open(&my_handle, NVS_STORAGE_NAME);
    count--;
    my_nvs_set_i16(my_handle, NVS_KEY_COUNT, count);
    my_nvs_erase_key(my_handle, key);
    my_nvs_commit_and_close(my_handle);

    int index = password_registry_get_index_by_id(id);
    for (int j = index; j < count; j++) {
        entries[j] = entries[j + 1];
    }
    ESP_LOGI(TAG, "Password Removed, total count %d", count);
}

password_entry_t* password_registry_get_entry_by_index(int i) {
    return i < count ? &entries[i] : NULL;
}

static int get_stored_passwords_count() {
    return count;
}

static char* get_name(int i) {
    return entries[i].name;
}

registry_api_t password_registry_common = {
    .get_name = get_name,
    .get_count = get_stored_passwords_count,
};
