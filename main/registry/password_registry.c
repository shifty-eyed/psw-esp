#include "item_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static password_entry* passwords;
static int count;

void password_registry_load() {
    count = 10;
    passwords = (password_entry *)malloc(count * sizeof(password_entry));

    for (int i = 0; i < count; i++) {
        sprintf(passwords[i].name, "name %d", i);
        sprintf(passwords[i].password, "password%d", i);
    }
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
