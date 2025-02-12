#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "keyboard.h"

#include "registry/item_registry.h"

static const char *TAG = "PSW_DIALOG";

static password_entry_t current_editing_entry;

static lv_obj_t* dialog = NULL;
static lv_obj_t* save_button = NULL;
static lv_obj_t* cancel_button = NULL;
static lv_obj_t* delete_button = NULL;
static lv_obj_t* input_password = NULL;
static lv_obj_t* input_name = NULL;
static lv_obj_t* kb = NULL;
static lv_obj_t* tabview = NULL;
static lv_obj_t* tab_generate = NULL;
static lv_obj_t* tab_type = NULL;
static lv_obj_t* password_length_slider = NULL;
static lv_obj_t* password_length_label = NULL;
static lv_obj_t* generate_use_numbers = NULL;
static lv_obj_t* generate_use_symbols_set1 = NULL;
static lv_obj_t* generate_use_symbols_set2 = NULL;
static lv_obj_t* generate_button = NULL;

static const lv_color_t delete_button_bg_color = { .red = 180, .green = 40, .blue = 40 };
static const lv_color_t tab_bg_color = { .red = 180, .green = 180, .blue = 180 };


static void apply_styles() {
    lv_obj_set_style_bg_color(dialog, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);

    lv_obj_set_style_bg_color(input_name, lv_color_black(), 0);
    lv_obj_set_style_text_color(input_name, lv_color_white(), 0);
    lv_obj_set_style_bg_color(input_password, lv_color_black(), 0);
    lv_obj_set_style_text_color(input_password, lv_color_white(), 0);

    lv_obj_set_style_bg_color(kb, lv_color_black(), 0);
    lv_obj_set_style_text_color(kb, lv_color_white(), 0);
    
    lv_obj_set_style_bg_color(delete_button, delete_button_bg_color, 0);
    
}

static void cancel_dialog_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Close dialog");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
}

static void delete_entry_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete password");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    password_registry_remove_password(current_editing_entry.id);
}

static void save_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new password");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    
    const char * password = lv_textarea_get_text(input_password);
    strcpy(current_editing_entry.password, password);
    strcpy(current_editing_entry.name, "New Password");

    if (current_editing_entry.id == 0) {
        password_registry_add_new_password(&current_editing_entry);
    } else {
        password_registry_update_password(&current_editing_entry);
    }
    
    //int index = device_registry_get_index_by_name(name);
    //ui_on_new_device_saved(index);
}

static void btn_set_enabled(lv_obj_t *btn, bool enabled) {
    if (enabled) {
        lv_obj_remove_state(btn, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
}

static void evaluate_save_button_state() {
    const char *text = lv_textarea_get_text(input_password);
    bool text_entered = (text != NULL) && (strlen(text) > 0);
    text = lv_textarea_get_text(input_name);
    bool name_entered = (text != NULL) && (strlen(text) > 0);

    btn_set_enabled(save_button, text_entered && name_entered);
    btn_set_enabled(delete_button, current_editing_entry.id != 0);
}

static void text_input_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if(kb != NULL) lv_keyboard_set_textarea(kb, ta);
        if (lv_tabview_get_tab_active(tabview) != 1) {
            lv_tabview_set_active(tabview, 1, LV_ANIM_ON);
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        evaluate_save_button_state();
    }
}

void edit_password_dialog_show(password_entry_t* initial_value) {
    if (initial_value != NULL) {
        current_editing_entry = *initial_value;
    } else {
        current_editing_entry.id = 0;
        current_editing_entry.name[0] = '\0';
        current_editing_entry.password[0] = '\0';    
    }
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(input_password, current_editing_entry.password);
    lv_textarea_set_text(input_password, "");
    evaluate_save_button_state();
}

static void slider_event_cb(lv_event_t *e) {
    char buf[4];
    sprintf(buf, "%d", (int)lv_slider_get_value(password_length_slider));
    lv_label_set_text(password_length_label, buf);
}

static void generate_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Generate password");
    char password[128];
    int length = lv_slider_get_value(password_length_slider);
    bool use_numbers = lv_obj_get_state(generate_use_numbers) & LV_STATE_CHECKED;
    bool use_symbols_set1 = lv_obj_get_state(generate_use_symbols_set1) & LV_STATE_CHECKED;
    bool use_symbols_set2 = lv_obj_get_state(generate_use_symbols_set2) & LV_STATE_CHECKED;
    //generate_password(password, length, use_numbers, use_symbols_set1, use_symbols_set2);
    sprintf(password, "%d-%s-%s-%s", length, use_numbers ? "12" : "_", use_symbols_set1 ? "+=" : "_", use_symbols_set2 ? "<>" : "_");
    lv_textarea_set_text(input_password, password);
    evaluate_save_button_state();
}
//todo: fix layout, on text input selected attach kb and switch tab, 
void edit_password_dialog_init() {
    if (dialog != NULL) {
        return;
    }
    dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dialog, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);

    save_button = lv_button_create(dialog);
    lv_obj_add_event_cb(save_button, save_password_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_set_size(save_button, 85, 30);
    lv_obj_align(save_button, LV_ALIGN_TOP_LEFT, -15, -15);

    delete_button = lv_button_create(dialog);
    lv_obj_add_event_cb(delete_button, delete_entry_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(delete_button, LV_SYMBOL_TRASH, 0);
    lv_obj_set_size(delete_button, 70, 30);
    lv_obj_align(delete_button, LV_ALIGN_TOP_MID, -1, -17);

    cancel_button = lv_button_create(dialog);
    lv_obj_add_event_cb(cancel_button, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_set_size(cancel_button, 85, 30);
    lv_obj_align(cancel_button, LV_ALIGN_TOP_RIGHT, 15, -18);

    lv_obj_t* label_name = lv_label_create(dialog);
    lv_label_set_text(label_name, "Name:");
    lv_obj_align(label_name, LV_ALIGN_TOP_LEFT, 0, 20);

    input_name = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(input_name, "Entry name");
    lv_textarea_set_one_line(input_name, true);
    lv_obj_set_size(input_name, 180, 30);
    lv_obj_align(input_name, LV_ALIGN_TOP_RIGHT, 0, 20);
    lv_obj_set_style_pad_ver(input_name, 2, 0);
    lv_obj_add_event_cb(input_name, text_input_cb, LV_EVENT_ALL, NULL);

    input_password = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(input_password, "Generate or type password");
    lv_textarea_set_one_line(input_password, true);
    lv_obj_set_size(input_password, SCREEN_W - 10, 30);
    lv_obj_align(input_password, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_pad_ver(input_password, 2, 0);
    lv_obj_add_event_cb(input_password, text_input_cb, LV_EVENT_ALL, NULL);

    tabview = lv_tabview_create(dialog);
    lv_obj_set_style_bg_color(tabview, tab_bg_color, 0);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_TOP, 0);
    lv_tabview_set_tab_bar_size(tabview, 25);
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_BOTTOM);
    lv_obj_set_size(tabview, SCREEN_W, SCREEN_H / 2 + 25);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 20);

    tab_generate = lv_tabview_add_tab(tabview, "Generate");
    lv_obj_set_style_margin_all(tab_generate, 0, 0);
    lv_obj_set_style_pad_all(tab_generate, 2, 0);
    lv_obj_set_scroll_dir(tab_generate, LV_DIR_VER);

    generate_button = lv_button_create(tab_generate);
    lv_obj_set_style_bg_image_src(generate_button, LV_SYMBOL_REFRESH, 0);
    lv_obj_set_size(generate_button, 30, 40);
    lv_obj_align(generate_button, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(generate_button, generate_password_cb, LV_EVENT_CLICKED, NULL);
    
    password_length_slider = lv_slider_create(tab_generate);
    lv_obj_align(password_length_slider, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_slider_set_range(password_length_slider, 8, 32);
    lv_slider_set_value(password_length_slider, 20, LV_ANIM_OFF);
    lv_obj_add_event_cb(password_length_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(password_length_slider, 140, 20);
    lv_obj_set_style_height(password_length_slider, 5, 0);
    lv_obj_set_style_radius(password_length_slider, 8, LV_PART_KNOB);

    password_length_label = lv_label_create(tab_generate);
    lv_obj_align(password_length_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(password_length_label, 30, 20);
    lv_label_set_text(password_length_label, "Length: 20");

    generate_use_numbers = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_numbers, "Use numbers");
    lv_obj_align(generate_use_numbers, LV_ALIGN_TOP_LEFT, 10, 30);
    
    generate_use_symbols_set1 = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_symbols_set1, "Use -+=_!@#$%%*");
    lv_obj_align(generate_use_symbols_set1, LV_ALIGN_TOP_LEFT, 10, 60);

    generate_use_symbols_set2 = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_symbols_set2, "Use <>,.';:?/|\\(){}[]^");
    lv_obj_align(generate_use_symbols_set2, LV_ALIGN_TOP_LEFT, 10, 90);

    tab_type = lv_tabview_add_tab(tabview, "Type");
    lv_obj_remove_flag(tab_type, LV_OBJ_FLAG_SCROLLABLE);
    kb = lv_my_keyboard_create(tab_type);
    lv_obj_set_size(kb,  SCREEN_W, SCREEN_H / 2);
    lv_obj_align(kb, LV_ALIGN_CENTER, 0, 0);
    
    lv_keyboard_set_textarea(kb, input_password);

    apply_styles();

    //fix the error when connecting known device
}