#include "esp_log.h"
#include "item_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "PSW_REG";

#define NVS_STORAGE_NAME "psw_storage"
#define NVS_KEY_COUNT "psw_count"
#define NVS_KEY_FORMAT "psw_%d"

static password_entry_t* passwords;
static int count;


static int get_index_by_id(uint16_t id) {
    for (int i = 0; i < count; i++) {
        if (passwords[i].id == id) {
            return i;
        }
    }
    return -1;
}

void password_registry_load() {
    count = 5;
    passwords = (password_entry_t *)malloc(count * sizeof(password_entry_t));

    for (int i = 0; i < count; i++) {
        sprintf(passwords[i].name, "name %d", i);
        sprintf(passwords[i].password, "password%d", i);
        passwords[i].id = i + 1;
    }
}



void password_registry_add_new_password(password_entry_t* entry) {
    ESP_LOGI(TAG, "add_new_password: name=%s, val=%s", entry->name, entry->password);
}

void password_registry_update_password(password_entry_t* entry) {
    ESP_LOGI(TAG, "update_password: name=%s, val=%s", entry->name, entry->password);
}

void password_registry_remove_password(uint16_t id) {
    int i = get_index_by_id(id);
    ESP_LOGI(TAG, "remove_password: id=%d, index=%d, name=%s, val=%s", id, i, passwords[i].name, passwords[i].password);
}

password_entry_t* password_registry_get_entry_by_index(int i) {
    return i < count ? &passwords[i] : NULL;
}

static int get_stored_passwords_count() {
    return count;
}

static char* get_name(int i) {
    return passwords[i].name;
}

static void destroy() {
    free(passwords);
}

registry_api_t password_registry_common = {
    .get_name = get_name,
    .get_count = get_stored_passwords_count,
};
