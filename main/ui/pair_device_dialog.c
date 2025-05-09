#include <string.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_internal.h"
#include "keyboard.h"

#include "registry/item_registry.h"
#include "bt/my_bt.h"

static const char *TAG = "PAIR_DIALOG";

static lv_group_t* group_input_name = NULL;
static lv_obj_t* dialog = NULL;
static lv_obj_t* main_screen = NULL;
static lv_obj_t* spinner = NULL;
static lv_obj_t* status_label = NULL;
static lv_obj_t* save_button = NULL;
static lv_obj_t* cancel_button = NULL;
static lv_obj_t* input_name = NULL;
static lv_obj_t* kb = NULL;

static bool paired = false;

static void close_dialog() {
    lv_show(dialog, false);
    lv_show(main_screen, true);
    lv_group_focus_freeze(group_input_name, false);
}

static void cancel_dialog_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Close dialog");
    close_dialog();
    bt_stop_advertising();
}

static void save_password_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new device");
    close_dialog();
    
    
    const char * name = lv_textarea_get_text(input_name);
    strcpy(current_device.name, name);
    
    device_registry_add_new_device(&current_device);
    int index = device_registry_get_index_by_name(name);
    ui_on_new_device_saved(index);
}

static void evaluate_save_button_state() {
    const char *text = lv_textarea_get_text(input_name);
    bool text_entered = (text != NULL) && (strlen(text) > 0);
    if (paired && text_entered) {
        lv_obj_remove_state(save_button, LV_STATE_DISABLED);
    }
    else {
        lv_obj_add_state(save_button, LV_STATE_DISABLED);
    }
}

static void text_input_cb(lv_event_t *e) {
    if (e == NULL) {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);
    size_t len = strlen(lv_textarea_get_text(input_name));

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (len == 1) {
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
        } else if (len == 0) {
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);
        }
        evaluate_save_button_state();
    }
}

void pair_device_dialog_on_pairing_succeeded() {
    ESP_LOGI(TAG, "Pairing succeeded");
    lv_label_set_text(status_label, "Pairing succeeded");
    paired = true;
    lv_show(spinner, false);
    evaluate_save_button_state();
}

bool pair_device_dialog_is_open() {
    return !lv_obj_has_flag(dialog, LV_OBJ_FLAG_HIDDEN);
}

void pair_device_dialog_show() {
    paired = false;
    lv_label_set_text(status_label, "Pairing...");
    lv_show(spinner, true);
    lv_show(dialog, true);
    lv_show(main_screen, false);

    lv_group_focus_obj(input_name);
    lv_group_focus_freeze(group_input_name, true);

    lv_textarea_set_text(input_name, "");
    evaluate_save_button_state();
}

void pair_device_dialog_init(lv_obj_t* parent) {
    if (dialog != NULL) {
        return;
    }

    main_screen = parent;
    dialog = lv_obj_create(lv_screen_active());
    lv_obj_set_size(dialog, DIALOG_WIDTH, DIALOG_HEIGHT);
    lv_obj_center(dialog);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_black(), 0);
    lv_show(dialog, false);

    save_button = lv_button_create(dialog);
    lv_obj_add_event_cb(save_button, save_password_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_set_size(save_button, xcoord(170), ycoord(50));
    lv_obj_align(save_button, LV_ALIGN_TOP_LEFT, xcoord(10), ycoord(-20));
    lv_obj_set_style_bg_color(save_button, disabled_button_bg_color, LV_STATE_DISABLED);
    lv_obj_set_style_text_color(save_button, disabled_button_text_color, LV_STATE_DISABLED);

    cancel_button = lv_button_create(dialog);
    lv_obj_add_event_cb(cancel_button, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_set_size(cancel_button, xcoord(170), ycoord(50));
    lv_obj_align(cancel_button, LV_ALIGN_TOP_RIGHT, xcoord(-10), ycoord(-20));
    lv_obj_set_style_bg_color(cancel_button, cancel_button_bg_color, 0);

    spinner = lv_spinner_create(dialog, 1000, 60);
    lv_obj_align(spinner, LV_ALIGN_TOP_MID, xcoord(-50), ycoord(35));
    lv_obj_set_size(spinner, xcoord(40), ycoord(40));

    status_label = lv_label_create(dialog);
    lv_label_set_text(status_label, "Pairing...");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, xcoord(20), ycoord(55));

    input_name = lv_textarea_create(dialog);
    lv_obj_set_size(input_name, DIALOG_WIDTH-xcoord(40), ycoord(45));
    lv_obj_align(input_name, LV_ALIGN_TOP_MID, 0, ycoord(90));
    lv_textarea_set_placeholder_text(input_name, "New Device Name");
    lv_textarea_set_one_line(input_name, true);
    lv_textarea_set_cursor_click_pos(input_name, true);
    lv_obj_set_style_pad_ver(input_name, 4, 0);
    lv_obj_add_event_cb(input_name, text_input_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(input_name, text_input_bg_color, 0);

    kb = lv_my_keyboard_create(dialog);
    lv_obj_set_size(kb,  KEYBOARD_WIDTH, KEYBOARD_HEIGHT);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, KEYBOARD_OFFSET_V);
    
    lv_keyboard_set_textarea(kb, input_name);

    group_input_name = lv_group_create();
    lv_group_add_obj(group_input_name, input_name);

    //fix the error when connecting known device
}