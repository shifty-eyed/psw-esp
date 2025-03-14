#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_internal.h"
#include "keyboard.h"
#include "registry/item_registry.h"
#include "my_nvs.h"

static const char *TAG = "PSW_DIALOG";

static password_entry_t current_editing_entry;

static lv_group_t* group_input_password = NULL;
static lv_group_t* group_input_name = NULL;

static lv_obj_t* dialog = NULL;
static lv_obj_t* save_button = NULL;
static lv_obj_t* cancel_button = NULL;
static lv_obj_t* delete_button = NULL;

static lv_obj_t* input_password = NULL;
static lv_obj_t* input_name = NULL;
static lv_obj_t* container_input_name = NULL;
static lv_obj_t* container_input_password = NULL;

static lv_obj_t* kb = NULL;

static lv_obj_t* container_generate_settings = NULL;
static lv_obj_t* len_number_label = NULL;
static lv_obj_t* generate_use_numbers = NULL;
static lv_obj_t* generate_use_symbols_set1 = NULL;
static lv_obj_t* generate_use_symbols_set2 = NULL;
static lv_obj_t* generate_button = NULL;

static const lv_color_t delete_button_bg_color = { .red = 180, .green = 40, .blue = 40 };
static const lv_color_t title_text_color = { .red = 255, .green = 255, .blue = 255 };
static const lv_color_t text_input_bg_color = { .red = 60, .green = 60, .blue = 30 };

static void next_page_button_cb(lv_event_t *e);
static void prev_page_button_cb(lv_event_t *e);
static void close_generate_settings_cb(lv_event_t *e);


static void close_dialog() {
    lv_show(dialog, false);
    lv_group_focus_freeze(group_input_name, false);
    lv_group_focus_freeze(group_input_password, false);
}

static void cancel_dialog_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Close dialog");
    close_dialog();
}

static void delete_entry_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete password");
    close_dialog();
    password_registry_remove_password(current_editing_entry.id);
    ui_on_password_dialog_closed(-1);
}

static void save_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new password");
    close_dialog();
    
    strcpy(current_editing_entry.password, lv_textarea_get_text(input_password));
    strcpy(current_editing_entry.name, lv_textarea_get_text(input_name));

    if (current_editing_entry.id == 0) {
        password_registry_add_new_password(&current_editing_entry);
    } else {
        password_registry_update_password(&current_editing_entry);
    }
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
    if (e == NULL) {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    size_t len = strlen(lv_textarea_get_text(input_name));

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (ta == input_name) {
            if (len == 1) {
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            } else if (len == 0) {
                lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);
            }
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
    lv_show(dialog, true);
    lv_textarea_set_text(input_password, current_editing_entry.password);
    lv_textarea_set_text(input_name, current_editing_entry.name);

    prev_page_button_cb(NULL);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);

    evaluate_save_button_state();
}

static void gen_password_len_decrease_cb(lv_event_t *e) {
    int value = atoi(lv_textarea_get_text(len_number_label));
    char buf[4];
    itoa(value - 1, buf, 10);
    lv_textarea_set_text(len_number_label, buf);
}

static void gen_password_len_increase_cb(lv_event_t *e) {
    int value = atoi(lv_textarea_get_text(len_number_label));
    char buf[4];
    itoa(value + 1, buf, 10);
    lv_textarea_set_text(len_number_label, buf);
}

static void generate_button_long_press_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Long press: Generate password settings");
    lv_show(container_generate_settings, true);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_CLICKABLE);
}

static void close_generate_settings_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: next page");
    lv_show(container_generate_settings, false);
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_CLICKABLE);

    my_nvs_save_password_dialog_settings(atoi(lv_textarea_get_text(len_number_label)),
        lv_obj_get_state(generate_use_numbers) & LV_STATE_CHECKED,
        lv_obj_get_state(generate_use_symbols_set1) & LV_STATE_CHECKED,
        lv_obj_get_state(generate_use_symbols_set2) & LV_STATE_CHECKED);
}

static void generate_button_click_cb(lv_event_t *e) {
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

static void next_page_button_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: next page");
    lv_show(container_input_name, false);
    lv_show(container_input_password, true);
    lv_keyboard_set_textarea(kb, input_password);

    lv_group_focus_freeze(group_input_name, false);
    lv_group_focus_obj(input_password);
    lv_group_focus_freeze(group_input_password, true);
}

static void prev_page_button_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: prev page");
    lv_show(container_input_name, true);
    lv_show(container_input_password, false);
    lv_keyboard_set_textarea(kb, input_name);

    lv_group_focus_freeze(group_input_password, false);
    lv_group_focus_obj(input_name);
    lv_group_focus_freeze(group_input_name, true);
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

static void create_generate_settings_component() {
    lv_obj_t* label;
    int psw_length;
    char length_str[4];
    bool use_numbers, use_symbols_set1, use_symbols_set2;
    my_nvs_load_password_dialog_settings(&psw_length, &use_numbers, &use_symbols_set1, &use_symbols_set2);
    itoa(psw_length, length_str, 10);

    container_generate_settings = mylv_create_container(lv_screen_active(), SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_opa(container_generate_settings, LV_OPA_70, 0);

    lv_obj_t* settings_layout = mylv_create_container_flex(container_generate_settings, LV_FLEX_FLOW_COLUMN, SCREEN_W-25, 200);
    lv_obj_align(settings_layout, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_border_side(settings_layout, LV_BORDER_SIDE_FULL, 0);
    lv_obj_set_style_pad_all(settings_layout, 10, 0);
    lv_obj_set_style_bg_opa(settings_layout, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(settings_layout, lv_color_black(), 0);

    label = lv_label_create(settings_layout);
    lv_label_set_text(label, "GENERATOR SETTINGS");
    lv_obj_set_style_text_color(label, title_text_color, 0);
    lv_obj_set_style_margin_left(label, 10, 0);
    
    lv_obj_t* length_generate_bar = mylv_create_container_flex(settings_layout, LV_FLEX_FLOW_ROW, SCREEN_W, 35);
    lv_obj_set_style_pad_hor(length_generate_bar, 2, 0);
    lv_obj_set_flex_align(length_generate_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(length_generate_bar);
    lv_label_set_text(label, "Length:");

    lv_obj_t* len_decrease_button = lv_button_create(length_generate_bar);
    lv_obj_set_style_bg_image_src(len_decrease_button, LV_SYMBOL_MINUS, 0);
    lv_obj_set_size(len_decrease_button, 30, 30);
    lv_obj_add_event_cb(len_decrease_button, gen_password_len_decrease_cb, LV_EVENT_CLICKED, NULL);

    len_number_label = lv_textarea_create(length_generate_bar);
    lv_textarea_set_text(len_number_label, length_str);
    lv_textarea_set_one_line(len_number_label, true);
    lv_obj_remove_flag(len_number_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(len_number_label, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_size(len_number_label, 40, 30);
    lv_obj_set_style_pad_ver(len_number_label, 2, 0);

    lv_obj_t* len_increase_button = lv_button_create(length_generate_bar);
    lv_obj_set_style_bg_image_src(len_increase_button, LV_SYMBOL_PLUS, 0);
    lv_obj_set_size(len_increase_button, 30, 30);
    lv_obj_add_event_cb(len_increase_button, gen_password_len_increase_cb, LV_EVENT_CLICKED, NULL);

    generate_use_numbers = lv_checkbox_create(settings_layout);
    lv_checkbox_set_text(generate_use_numbers, "Use 0-9");
    lv_obj_add_state(generate_use_numbers, use_numbers ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
    
    generate_use_symbols_set1 = lv_checkbox_create(settings_layout);
    lv_checkbox_set_text(generate_use_symbols_set1, "Use: -+=_!@#$%*");
    lv_obj_add_state(generate_use_symbols_set1, use_symbols_set1 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);

    generate_use_symbols_set2 = lv_checkbox_create(settings_layout);
    lv_checkbox_set_text(generate_use_symbols_set2, "Use: >,.';:?/|({[^");
    lv_obj_add_state(generate_use_symbols_set2, use_symbols_set2 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);

    lv_obj_t* button_ok = lv_button_create(settings_layout);
    lv_obj_set_size(button_ok, 65, 30);
    lv_obj_set_style_margin_left(button_ok, 120, 0);
    lv_obj_add_event_cb(button_ok, close_generate_settings_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button_ok);
    lv_label_set_text(label, "OK");

    lv_show(container_generate_settings, false);
}

static void create_input_name_component() {
    lv_obj_t* label;
    container_input_name = mylv_create_container(dialog, SCREEN_W, 72);
    lv_obj_align(container_input_name, LV_ALIGN_TOP_MID, 0, 20);
    
    label = lv_label_create(container_input_name);
    lv_label_set_text(label, "Entry Name:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 15, 22);

    lv_obj_t* button_next = lv_button_create(container_input_name);
    lv_obj_set_size(button_next, 100, 30);
    lv_obj_align(button_next, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_add_event_cb(button_next, next_page_button_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button_next);
    lv_label_set_text(label, "Password >");

    input_name = lv_textarea_create(container_input_name);
    lv_obj_set_size(input_name, SCREEN_W-15, 40);
    lv_obj_align(input_name, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_textarea_set_one_line(input_name, true);
    lv_textarea_set_cursor_click_pos(input_name, true);
    lv_obj_set_style_pad_ver(input_name, 4, 0);
    lv_obj_add_event_cb(input_name, text_input_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_bg_color(input_name, text_input_bg_color, 0);

}

static void create_input_password_component() {
    lv_obj_t* label;

    container_input_password = mylv_create_container(dialog, SCREEN_W, 72);
    lv_obj_align(container_input_password, LV_ALIGN_TOP_MID, 0, 20);

    label = lv_label_create(container_input_password);
    lv_label_set_text(label, "Password:");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    
    lv_obj_t* button_prev = lv_button_create(container_input_password);
    lv_obj_set_size(button_prev, 65, 30);
    lv_obj_align(button_prev, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_pad_all(button_prev, 2, 0);
    lv_obj_add_event_cb(button_prev, prev_page_button_cb, LV_EVENT_CLICKED, NULL);
    label = lv_label_create(button_prev);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "< Name");

    generate_button = lv_button_create(container_input_password);
    lv_obj_set_size(generate_button, 35, 35);
    lv_obj_align(generate_button, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_obj_set_style_pad_all(generate_button, 2, 0);
    lv_obj_add_event_cb(generate_button, generate_button_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(generate_button, generate_button_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);
    lv_obj_set_style_bg_image_src(generate_button, LV_SYMBOL_REFRESH, 0);

    input_password = lv_textarea_create(container_input_password);
    lv_obj_set_size(input_password, SCREEN_W - 15, 40);
    lv_obj_align(input_password, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_textarea_set_placeholder_text(input_password, "Generate " LV_SYMBOL_REFRESH " or type");
    lv_textarea_set_one_line(input_password, true);
    lv_obj_set_style_pad_ver(input_password, 4, 0);
    lv_obj_add_event_cb(input_password, text_input_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(input_password, text_input_bg_color, 0);
}

void edit_password_dialog_init() {
    if (dialog != NULL) {
        return;
    }
    dialog = lv_obj_create(lv_screen_active());
    lv_obj_set_size(dialog, SCREEN_W, SCREEN_H);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_black(), 0);

    lv_show(dialog, false);

    create_top_bar_buttons();
    create_input_name_component();
    create_input_password_component();
    create_generate_settings_component();

    lv_show(container_input_name, true);
    lv_show(container_input_password, false);

    kb = lv_my_keyboard_create(dialog);
    lv_obj_set_size(kb,  SCREEN_W, SCREEN_H / 2 + 30);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 15);

    group_input_name = lv_group_create();
    lv_group_add_obj(group_input_name, input_name);

    group_input_password = lv_group_create();
    lv_group_add_obj(group_input_password, input_password);

}