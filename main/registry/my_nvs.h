#ifndef MY_NVS_H
#define MY_NVS_H

#include "nvs.h"

void my_nvs_open(nvs_handle_t *my_handle, const char *namespace);
void my_nvs_commit_and_close(nvs_handle_t my_handle);
void my_nvs_erase_key(nvs_handle_t my_handle, const char *key);
void my_nvs_set_i16(nvs_handle_t my_handle, const char *key, int16_t value);
int16_t my_nvs_get_i16(nvs_handle_t my_handle, const char *key);


#endif // MY_NVS_H