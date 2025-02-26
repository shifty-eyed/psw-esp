#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "keyboard.h"
#include "registry/item_registry.h"
#include "my_nvs.h"

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
static lv_obj_t* len_decrease_button = NULL;
static lv_obj_t* len_increase_button = NULL;
static lv_obj_t* len_number_label = NULL;
static lv_obj_t* generate_use_numbers = NULL;
static lv_obj_t* generate_use_symbols_set1 = NULL;
static lv_obj_t* generate_use_symbols_set2 = NULL;
static lv_obj_t* generate_button = NULL;

static const lv_color_t delete_button_bg_color = { .red = 180, .green = 40, .blue = 40 };

static void cancel_dialog_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Close dialog");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
}

static void delete_entry_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete password");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    password_registry_remove_password(current_editing_entry.id);
    ui_on_password_dialog_closed(-1);
}

static void save_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new password");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    
    strcpy(current_editing_entry.password, lv_textarea_get_text(input_password));
    strcpy(current_editing_entry.name, lv_textarea_get_text(input_name));

    if (current_editing_entry.id == 0) {
        password_registry_add_new_password(&current_editing_entry);
    } else {
        password_registry_update_password(&current_editing_entry);
    }
    my_nvs_save_password_dialog_settings(atoi(lv_textarea_get_text(len_number_label)),
        lv_obj_get_state(generate_use_numbers) & LV_STATE_CHECKED,
        lv_obj_get_state(generate_use_symbols_set1) & LV_STATE_CHECKED,
        lv_obj_get_state(generate_use_symbols_set2) & LV_STATE_CHECKED);
    
    int index = password_registry_get_index_by_id(current_editing_entry.id);
    ui_on_password_dialog_closed(index);
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
        lv_keyboard_set_textarea(kb, ta);
        if (lv_tabview_get_tab_active(tabview) != 1) {
            lv_tabview_set_active(tabview, 1, LV_ANIM_ON);
        }
    }
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (ta == input_name && strlen(lv_textarea_get_text(input_name)) == 1) {
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
        }
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
    lv_textarea_set_text(input_name, current_editing_entry.name);

    lv_tabview_set_active(tabview, 1, LV_ANIM_OFF);
    lv_keyboard_set_textarea(kb, input_name);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);

    evaluate_save_button_state();
}

static void len_change_cb(lv_event_t *e) {
    lv_obj_t * ta = lv_event_get_target(e);
    int value = atoi(lv_textarea_get_text(len_number_label));
    value += (ta == len_decrease_button) ? -1 : 1;
    char buf[4];
    sprintf(buf, "%d", value);
    lv_textarea_set_text(len_number_label, buf);
}

static void generate_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Generate password");
    char password[128];
    int length = atoi(lv_textarea_get_text(len_number_label));
    bool use_numbers = lv_obj_get_state(generate_use_numbers) & LV_STATE_CHECKED;
    bool use_symbols_set1 = lv_obj_get_state(generate_use_symbols_set1) & LV_STATE_CHECKED;
    bool use_symbols_set2 = lv_obj_get_state(generate_use_symbols_set2) & LV_STATE_CHECKED;
    password_registry_generate_password(password, length, use_numbers, use_symbols_set1, use_symbols_set2);
    lv_textarea_set_text(input_password, password);
    evaluate_save_button_state();
}

static void create_top_bar_buttons() {
    lv_obj_t* top_bar = lv_obj_create(dialog);
    lv_obj_set_size(top_bar, SCREEN_W, 30);
    lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, -18);
    lv_obj_set_style_border_side(top_bar, LV_BORDER_SIDE_NONE, 0);

    //lv_obj_set_layout(top_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_row(top_bar, 0, 0);
    lv_obj_set_style_pad_column(top_bar, 0, 0);
    lv_obj_set_style_pad_all(top_bar, 0, 0);
    lv_obj_remove_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);

    save_button = lv_button_create(top_bar);
    lv_obj_add_event_cb(save_button, save_password_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_set_size(save_button, SCREEN_W/3 + 5, 30);

    delete_button = lv_button_create(top_bar);
    lv_obj_add_event_cb(delete_button, delete_entry_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(delete_button, LV_SYMBOL_TRASH, 0);
    lv_obj_set_size(delete_button, SCREEN_W/3 - 5, 30);
    lv_obj_set_style_bg_color(delete_button, delete_button_bg_color, 0);

    cancel_button = lv_button_create(top_bar);
    lv_obj_add_event_cb(cancel_button, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_set_size(cancel_button, SCREEN_W/3 + 5, 30);
}

static void create_tab_generate() {
    int psw_length;
    char length_str[4];
    bool use_numbers, use_symbols_set1, use_symbols_set2;
    my_nvs_load_password_dialog_settings(&psw_length, &use_numbers, &use_symbols_set1, &use_symbols_set2);
    sprintf(length_str, "%d", psw_length);

    tab_generate = lv_tabview_add_tab(tabview, "Generate");
    lv_obj_set_style_margin_all(tab_generate, 0, 0);
    lv_obj_set_style_pad_hor(tab_generate, 15, 0);
    lv_obj_set_scroll_dir(tab_generate, LV_DIR_VER);
    lv_obj_set_flex_flow(tab_generate, LV_FLEX_FLOW_COLUMN);

    generate_button = lv_button_create(tab_generate);
    //lv_obj_set_style_bg_image_src(generate_button, LV_SYMBOL_REFRESH, 0);
    lv_obj_set_size(generate_button, 120, 30);
    lv_obj_set_style_margin_left(generate_button, (SCREEN_W - 116)/2 - 15, 0);
    lv_obj_add_event_cb(generate_button, generate_password_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* label_generate = lv_label_create(generate_button);
    lv_label_set_text(label_generate, LV_SYMBOL_REFRESH " Generate");

        lv_obj_t* length_generate_bar = lv_obj_create(tab_generate);
        lv_obj_set_size(length_generate_bar, SCREEN_W, 35);
        lv_obj_set_flex_flow(length_generate_bar, LV_FLEX_FLOW_ROW);
        lv_obj_remove_flag(length_generate_bar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_hor(length_generate_bar, 2, 0);
        lv_obj_set_style_bg_opa(length_generate_bar, LV_OPA_0, 0);
        lv_obj_set_style_border_side(length_generate_bar, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_flex_align(length_generate_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* len_label = lv_label_create(length_generate_bar);
        lv_label_set_text(len_label, "Length:");

        len_decrease_button = lv_button_create(length_generate_bar);
        lv_obj_set_style_bg_image_src(len_decrease_button, LV_SYMBOL_MINUS, 0);
        lv_obj_set_size(len_decrease_button, 30, 30);
        lv_obj_add_event_cb(len_decrease_button, len_change_cb, LV_EVENT_CLICKED, NULL);
    
        len_number_label = lv_textarea_create(length_generate_bar);
        lv_textarea_set_text(len_number_label, length_str);
        lv_textarea_set_one_line(len_number_label, true);
        lv_obj_remove_flag(len_number_label, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(len_number_label, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_size(len_number_label, 40, 30);
        lv_obj_set_style_pad_ver(len_number_label, 2, 0);

        len_increase_button = lv_button_create(length_generate_bar);
        lv_obj_set_style_bg_image_src(len_increase_button, LV_SYMBOL_PLUS, 0);
        lv_obj_set_size(len_increase_button, 30, 30);
        lv_obj_add_event_cb(len_increase_button, len_change_cb, LV_EVENT_CLICKED, NULL);


    generate_use_numbers = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_numbers, "Use numbers");
    lv_obj_add_state(generate_use_numbers, use_numbers ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
    
    generate_use_symbols_set1 = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_symbols_set1, "Use symbols: -+=_!@#$%*");
    lv_obj_add_state(generate_use_symbols_set1, use_symbols_set1 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);

    generate_use_symbols_set2 = lv_checkbox_create(tab_generate);
    lv_checkbox_set_text(generate_use_symbols_set2, "Use symbols: >,.';:?/|({[^");
    lv_obj_add_state(generate_use_symbols_set2, use_symbols_set2 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
}


void edit_password_dialog_init() {
    if (dialog != NULL) {
        return;
    }
    dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dialog, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_black(), 0);

    create_top_bar_buttons();

    lv_obj_t* label_name = lv_label_create(dialog);
    lv_label_set_text(label_name, "Name:");
    lv_obj_align(label_name, LV_ALIGN_TOP_LEFT, -1, 28);

    input_name = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(input_name, "Entry name");
    lv_textarea_set_one_line(input_name, true);
    lv_obj_set_size(input_name, 160, 30);
    lv_obj_align(input_name, LV_ALIGN_TOP_RIGHT, 0, 22);
    lv_obj_set_style_pad_ver(input_name, 3, 0);
    lv_obj_add_event_cb(input_name, text_input_cb, LV_EVENT_ALL, NULL);

    input_password = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(input_password, "Generate or type password");
    lv_textarea_set_one_line(input_password, true);
    lv_obj_set_size(input_password, SCREEN_W - 20, 35);
    lv_obj_align(input_password, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_pad_ver(input_password, 3, 0);
    lv_obj_add_event_cb(input_password, text_input_cb, LV_EVENT_ALL, NULL);

    tabview = lv_tabview_create(dialog);
    //lv_obj_set_style_bg_color(tabview, tab_bg_color, 0);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_TOP, 0);
    lv_tabview_set_tab_bar_size(tabview, 25);
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_BOTTOM);
    lv_obj_set_size(tabview, SCREEN_W, SCREEN_H / 2 + 25);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 15);

    create_tab_generate();

    tab_type = lv_tabview_add_tab(tabview, "Type");
    lv_obj_remove_flag(tab_type, LV_OBJ_FLAG_SCROLLABLE);
    kb = lv_my_keyboard_create(tab_type);
    lv_obj_set_size(kb,  SCREEN_W, SCREEN_H / 2);
    lv_obj_align(kb, LV_ALIGN_CENTER, 0, 0);

}