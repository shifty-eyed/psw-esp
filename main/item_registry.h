#ifndef PASSWORD_REGISTRY_H
#define PASSWORD_REGISTRY_H

#include <stdint.h>
#include "esp_bt_defs.h"

#define NAME_MAX_LENGTH 32
#define PASSWORD_MAX_LENGTH 128

typedef struct {
    char name[NAME_MAX_LENGTH];
    char password[PASSWORD_MAX_LENGTH];
} password_entry;

typedef struct {
    void (*remove)(int i);
    char* (*get_name)(int i);
    int (*get_count)();
    void (*load)();
    void (*destroy)();
} registry_api_t;

extern registry_api_t password_registry_common;
extern registry_api_t device_registry_common;

/* Device related */

typedef struct {
    char name[NAME_MAX_LENGTH];
    esp_bd_addr_t addr;
    esp_ble_addr_type_t addr_type;
} device_entry_t;

void device_add(const device_entry_t* device);
device_entry_t* device_get(int i);


#endif // PASSWORD_REGISTRY_H
