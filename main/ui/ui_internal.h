#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

#define LVGL8 

#include "item_registry.h"
#include "lvgl9to8.h"

extern 
lv_color_t row_bg_color1, row_bg_color2, row_bg_selected, row_text_color, 
    delete_button_bg_color,
    cancel_button_bg_color,
    disabled_button_bg_color,
    disabled_button_text_color,
    title_text_color, text_input_bg_color;

void delayed_menu_hide(lv_obj_t* menu);
bool popup_menu_visible(lv_obj_t* menu);

lv_obj_t* popup_menu_create(lv_obj_t* parent, int num_items);
lv_obj_t* popup_menu_add_button(lv_obj_t* menu, const char* text,  const char* symbol, lv_event_cb_t cb);

void pair_device_dialog_init(lv_obj_t* parent);
bool pair_device_dialog_is_open();
void pair_device_dialog_show();
void pair_device_dialog_on_pairing_succeeded();

void edit_password_dialog_init(lv_obj_t* parent);
void edit_password_dialog_show(password_entry_t* initial_value);


void backup_dialog_show(lv_obj_t *parent);

void show_toast(const char *message, bool is_error);
void show_spinner(bool show);

void init_color_theme();
void init_battery_indicator();

void lv_enable(lv_obj_t *obj, bool enabled);
void lv_show(lv_obj_t *obj, bool visible);
lv_obj_t* mylv_create_container_flex(lv_obj_t *parent, lv_flex_flow_t flow, int32_t width, int32_t height);
lv_obj_t* mylv_create_container(lv_obj_t* parent, int32_t width, int32_t height);

lv_coord_t xcoord(lv_coord_t value);
lv_coord_t ycoord(lv_coord_t value);

#endif // UI_INTERNAL_H