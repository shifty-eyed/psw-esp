#include "item_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static device_entry* devices;
static int count;

static void load() {
    count = 3;
    devices = (device_entry *)malloc(count * sizeof(device_entry));

    for (int i = 0; i < count; i++) {
        sprintf(devices[i].name, "Device%d", i);
        sprintf(devices[i].mac, "mac %d", i);
    }
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

registry_api device_registry = {
    .get_name = get_name,
    .get_count = get_stored_count,
    .load = load,
    .destroy = destroy
};
