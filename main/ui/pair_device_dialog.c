
#include "lvgl.h"
#include "ui.h"
#include "ui_styles.h"
#include "esp_log.h"


static const char *TAG = "PAIR_DIALOG";

static lv_obj_t* dialog = NULL;
static lv_obj_t *spinner = NULL;
static lv_obj_t *status_label = NULL;
static lv_obj_t *save_button = NULL;
static lv_obj_t *cancel_button = NULL;
static lv_obj_t *name_input = NULL;

static ui_api_callbacks_t* api_callbacks;

static bool paired = false;

//API: dialog actions: cancel_pairing(), save_new_device()
//API: dialog receives: pair_device_dialog_on_pairing_succeeded(addr, addr_type)

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
    bool text_entered = lv_textarea_get_text_length(name_input) > 0;
    if (paired && text_entered) {
        lv_obj_remove_state(save_button, LV_STATE_DISABLED);
    } else {
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
        return dialog;
    }
    api_callbacks = callbacks;
    dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dialog, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(dialog, LV_OBJ_FLAG_HIDDEN);
    style_pair_device_dialog(dialog);

    name_input = lv_textarea_create(dialog);
    lv_textarea_set_placeholder_text(name_input, "New Device Name");
    lv_obj_set_size(name_input, 200, 40);
    lv_obj_align(name_input, LV_ALIGN_TOP_MID, 0, 20);

    status_label = lv_label_create(dialog);
    lv_label_set_text(status_label, "Pairing...");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, -40, 80);

    spinner = lv_spinner_create(dialog);
    lv_obj_align_to(spinner, status_label, LV_ALIGN_OUT_RIGHT_MID, 0, 10);

    save_button = lv_button_create(dialog);
    lv_obj_set_event_cb(save_button, save_new_device_cb);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_align(save_button, LV_ALIGN_BOTTOM_LEFT, 20, -20);

    cancel_button = lv_button_create(dialog);
    lv_obj_set_event_cb(cancel_button, cancel_dialog_cb);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_align(cancel_button, LV_ALIGN_BOTTOM_RIGHT, -20, -20);

    return dialog;
}