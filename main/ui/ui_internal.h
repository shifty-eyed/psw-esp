#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

#define LVGL8 

#include "item_registry.h"
#include "lvgl9to8.h"
#include "style.h"

void pair_device_dialog_init();
bool pair_device_dialog_is_open();
void pair_device_dialog_show();
void pair_device_dialog_on_pairing_succeeded();

void edit_password_dialog_init();
void edit_password_dialog_show(password_entry_t* initial_value);

void show_toast(const char *message, bool is_error);
void show_spinner(bool show);

void lv_enable(lv_obj_t *obj, bool enabled);
void lv_show(lv_obj_t *obj, bool visible);
lv_obj_t* mylv_create_container_flex(lv_obj_t *parent, lv_flex_flow_t flow, int32_t width, int32_t height);
lv_obj_t* mylv_create_container(lv_obj_t* parent, int32_t width, int32_t height);

#endif // UI_INTERNAL_H