#include "ui.h"
#include "ui_internal.h"


lv_color_t row_bg_color1, row_bg_color2, row_bg_selected, row_text_color, 
    delete_button_bg_color,
    cancel_button_bg_color,
    disabled_button_bg_color,
    disabled_button_text_color,
    title_text_color, text_input_bg_color;

static lv_obj_t *toast = NULL;
static lv_timer_t *timer = NULL;
static lv_obj_t* spinner;

void init_color_theme() {
    row_bg_color1 = lv_color_make(10, 10, 15);
    row_bg_color2 = lv_color_make(15, 15, 20);
    row_bg_selected = lv_color_make(40, 70, 200);
    row_text_color = lv_color_make(240, 240, 240);
    delete_button_bg_color = lv_color_make(180, 40, 40);
    cancel_button_bg_color = lv_color_make(40, 40, 80);
    title_text_color = lv_color_make(255, 255, 255);
    text_input_bg_color = lv_color_make(60, 60, 30);

    disabled_button_bg_color = lv_color_make(40, 40, 80);
    disabled_button_text_color = lv_color_black();

    lv_theme_t *theme = lv_theme_default_init(
        lv_disp_get_default(),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_DEEP_ORANGE),
        true,  // Dark mode
        LV_FONT_DEFAULT);
        
    lv_disp_set_theme(lv_disp_get_default(), theme);
}

lv_coord_t xcoord(lv_coord_t value) {
    return (lv_coord_t)(value * COORD_RATIO_X);
}

lv_coord_t ycoord(lv_coord_t value) {
    return (lv_coord_t)(value * COORD_RATIO_Y);
}


lv_obj_t* mylv_create_container_flex(lv_obj_t* parent, lv_flex_flow_t flow, int32_t width, int32_t height) {
    lv_obj_t* result = mylv_create_container(parent, width, height);
    lv_obj_set_flex_flow(result, flow);
    return result;
}

lv_obj_t* mylv_create_container(lv_obj_t* parent, int32_t width, int32_t height) {
    lv_obj_t* result = lv_obj_create(parent);
    lv_obj_set_style_pad_all(result, 0, 0);
    lv_obj_set_style_border_side(result, LV_BORDER_SIDE_NONE, 0);
    lv_obj_remove_flag(result, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(result, LV_OPA_0, 0);
    if (width > 0) {
        lv_obj_set_width(result, width);
    }
    if (height > 0) {
        lv_obj_set_height(result, height);
    }
    return result;
}

void lv_enable(lv_obj_t *obj, bool enabled) {
    if (enabled) {
        lv_obj_remove_state(obj, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }
}

void lv_show(lv_obj_t *obj, bool visible) {
    if (visible) {
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    }
}

static void toast_timer_cb(lv_timer_t *t) {
    lv_obj_delete(toast);
    lv_timer_delete(timer);
    toast = NULL;
    timer = NULL;
}

void show_toast(const char *message, bool is_error) {
    if (toast) {
        return;
    }
    toast = lv_obj_create(lv_scr_act());
    lv_obj_set_size(toast, 200, 50);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_bg_color(toast, lv_palette_main(is_error ? LV_PALETTE_RED : LV_PALETTE_BLUE), 0);
    lv_obj_set_style_radius(toast, 10, 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_set_style_text_color(toast, lv_color_white(), 0);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *label = lv_label_create(toast);
    lv_obj_center(label);

    lv_label_set_text(lv_obj_get_child(toast, 0), message);

    if (timer) lv_timer_del(timer);
    timer = lv_timer_create(toast_timer_cb, TOAST_DURATION, toast);
    lv_timer_set_repeat_count(timer, 1);
}

void show_spinner(bool show) {
    if (!spinner) {
        spinner = lv_spinner_create(lv_screen_active(), 1000, 60);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(spinner, SPINNER_SIZE, SPINNER_SIZE);
    }
    if (show) {
        lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    }
}
