#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_internal.h"
#include "registry/item_registry.h"
#include "my_nvs.h"

static const char *TAG = "BACKUP_DIALOG";


static lv_obj_t* dialog = NULL;
static lv_obj_t* main_screen = NULL;
static lv_obj_t* status_label = NULL;
static lv_obj_t* key_label = NULL;
static lv_obj_t* save_button = NULL;
static lv_obj_t* cancel_button = NULL;
static lv_obj_t* new_key_button = NULL;

static bool ready = false;


static void close_dialog() {
    lv_show(dialog, false);
    lv_show(main_screen, true);
}

static void save_backup_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Save new device");
    close_dialog();
    
    //TODO: Implement backup logic
}

static void evaluate_save_button_state() {
    if (ready) {
        lv_obj_remove_state(save_button, LV_STATE_DISABLED);
    }
    else {
        lv_obj_add_state(save_button, LV_STATE_DISABLED);
    }
}


static void dialog_init(lv_obj_t *parent) {
    main_screen = parent;
    dialog = lv_obj_create(lv_screen_active());
    lv_obj_set_size(dialog, DIALOG_WIDTH, DIALOG_HEIGHT);
    lv_obj_center(dialog);
    lv_obj_remove_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(dialog, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_black(), 0);

    save_button = lv_button_create(dialog);
    lv_obj_add_event_cb(save_button, save_backup_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(save_button, LV_SYMBOL_SAVE, 0);
    lv_obj_set_size(save_button, xcoord(170), ycoord(50));
    lv_obj_align(save_button, LV_ALIGN_TOP_LEFT, xcoord(10), ycoord(-20));
    lv_obj_set_style_bg_color(save_button, disabled_button_bg_color, LV_STATE_DISABLED);
    lv_obj_set_style_text_color(save_button, disabled_button_text_color, LV_STATE_DISABLED);

    cancel_button = lv_button_create(dialog);
    lv_obj_add_event_cb(cancel_button, close_dialog, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(cancel_button, LV_SYMBOL_CLOSE, 0);
    lv_obj_set_size(cancel_button, xcoord(170), ycoord(50));
    lv_obj_align(cancel_button, LV_ALIGN_TOP_RIGHT, xcoord(-10), ycoord(-20));
    lv_obj_set_style_bg_color(cancel_button, cancel_button_bg_color, 0);

}


void backup_dialog_show(lv_obj_t *parent) {
    if (dialog == NULL) {
        dialog_init(parent);
    }
    ESP_LOGI(TAG, "backup_dialog_show()");
    lv_show(dialog, true);
    lv_show(main_screen, false);
    evaluate_save_button_state();
}