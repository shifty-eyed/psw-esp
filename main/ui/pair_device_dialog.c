#include <string.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "keyboard.h"

static const char *TAG = "PAIR_DIALOG";

static lv_obj_t* dialog = NULL;
static lv_obj_t* spinner = NULL;
static lv_obj_t* status_label = NULL;
static lv_obj_t* save_button = NULL;
static lv_obj_t* cancel_button = NULL;
static lv_obj_t* name_input = NULL;
static lv_obj_t* kb = NULL;

static ui_api_callbacks_t *api_callbacks;

static bool paired = false;

const lv_color_t cancel_button_bg_color = { .red = 180, .green = 40, .blue = 40 };

static void apply_styles() {
    lv_obj_set_style_bg_color(dialog, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);

    //make dark background for kb and input
    lv_obj_set_style_bg_color(name_input, lv_color_black(), 0);
    lv_obj_set_style_text_color(name_input, lv_color_white(), 0);
    lv_obj_set_style_bg_color(kb, lv_color_black(), 0);
    lv_obj_set_style_text_color(kb, lv_color_white(), 0);
    lv_obj_set_style_text_color(status_label, lv_color_white(), 0);
    
    lv_obj_set_style_bg_color(cancel_button, cancel_button_bg_color, 0);
    
}

static void cancel_dialog_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Close dialog");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    api_callbacks->cancel_pairing();
}

static void save_new_device_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new device");
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    api_callbacks->save_new_device(lv_textarea_get_text(name_input));
}

static void evaluate_save_button_state() {
    const char *text = lv_textarea_get_text(name_input);
    bool text_entered = (text != NULL) && (strlen(text) > 0);
    if (paired && text_entered)
    {
        lv_obj_remove_state(save_button, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_add_state(save_button, LV_STATE_DISABLED);
    }
}

void pair_device_dialog_on_pairing_succeeded() {
    ESP_LOGI(TAG, "Pairing succeeded");
    lv_label_set_text(status_label, "Pairing succeeded");
    paired = true;
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    evaluate_save_button_state();
}

bool pair_device_dialog_is_open() {
    return !lv_obj_has_flag(dialog, LV_OBJ_FLAG_HIDDEN);
}

void pair_device_dialog_show() {
    paired = false;
    lv_label_set_text(status_label, "Pairing...");
    lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(name_input, "");
    evaluate_save_button_state();
}

void pair_device_dialog_init(ui_api_callbacks_t *callbacks) {
    if (dialog != NULL) {
        return;
    }
    api_callbacks = callbacks;
    dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dialog, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);

    save_button = lv_button_create(dialog);
    lv_obj_add_event_cb(save_button, save_new_device_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_set_size(save_button, 80, 40);
    lv_obj_align(save_button, LV_ALIGN_TOP_LEFT, 0, -20);

    cancel_button = lv_button_create(dialog);
    lv_obj_add_event_cb(cancel_button, cancel_dialog_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_set_size(cancel_button, 80, 40);
    lv_obj_align(cancel_button, LV_ALIGN_TOP_RIGHT, 0, -20);

    spinner = lv_spinner_create(dialog);
    lv_obj_align(spinner, LV_ALIGN_TOP_LEFT, 20, 30);
    lv_obj_set_size(spinner, 30, 30);

    status_label = lv_label_create(dialog);
    lv_label_set_text(status_label, "Pairing...");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 40);

    name_input = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(name_input, "New Device Name");
    lv_textarea_set_one_line(name_input, true);
    lv_obj_set_size(name_input, SCREEN_W - 20, 40);
    lv_obj_align(name_input, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_add_event_cb(name_input, evaluate_save_button_state, LV_EVENT_VALUE_CHANGED, NULL);

    kb = lv_my_keyboard_create(dialog);
    lv_obj_set_size(kb,  SCREEN_W, SCREEN_H / 2);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 20);
    
    lv_keyboard_set_textarea(kb, name_input);

    apply_styles();

    //fix the error when connecting known device
}