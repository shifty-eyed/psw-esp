#ifndef PASSWORD_REGISTRY_H
#define PASSWORD_REGISTRY_H

#include <stdint.h>

#define NAME_MAX_LENGTH 32
#define PASSWORD_MAX_LENGTH 32

typedef struct {
    char name[NAME_MAX_LENGTH];
    char mac[PASSWORD_MAX_LENGTH];
} device_entry;

typedef struct {
    char name[NAME_MAX_LENGTH];
    char password[PASSWORD_MAX_LENGTH];
} password_entry;

typedef struct {
    //void (*add)(generic_entry *entry);
    //void (*remove)(uint16_t id);
    //void (*update)(generic_entry *entry);
    //generic_entry* (*get)(uint16_t id);
    char* (*get_name)(int i);
    int (*get_count)();
    void (*load)();
    void (*destroy)();
} registry_api;

extern registry_api password_registry;
extern registry_api device_registry;


#endif // PASSWORD_REGISTRY_H
