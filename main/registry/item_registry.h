#ifndef PASSWORD_REGISTRY_H
#define PASSWORD_REGISTRY_H

#include <stdint.h>
#include "esp_bt_defs.h"

#define NAME_MAX_LENGTH 32
#define PASSWORD_MAX_LENGTH 128

typedef struct {
    char* (*get_name)(int i);
    int (*get_count)();
} registry_api_t;

extern registry_api_t password_registry_common;
extern registry_api_t device_registry_common;

/* Device related */

typedef struct {
    uint16_t id;
    char name[NAME_MAX_LENGTH];
    esp_bd_addr_t addr;
    esp_ble_addr_type_t addr_type;
} device_entry_t;

extern device_entry_t current_device;

void device_registry_load();
void device_registry_add_new_device(device_entry_t* device);
void device_registry_remove_device(int i);

device_entry_t* device_registry_get_by_index(int i);
int device_registry_get_index_by_name(const char* name);
device_entry_t* device_registry_get_by_address(const esp_bd_addr_t addr);

/* Password related */
typedef struct {
    uint16_t id;
    char name[NAME_MAX_LENGTH];
    char password[PASSWORD_MAX_LENGTH];
} password_entry_t;

void password_registry_load();
void password_registry_add_new_password(password_entry_t* entry);
void password_registry_update_password(password_entry_t* entry);
void password_registry_remove_password(uint16_t id);
password_entry_t* password_registry_get_entry_by_index(int i);


#endif // PASSWORD_REGISTRY_H
