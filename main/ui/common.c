#include "ui.h"
#include "ui_internal.h"


lv_color_t row_bg_color1, row_bg_color2, row_bg_selected, row_text_color, 
    delete_button_bg_color,
    cancel_button_bg_color,
    disabled_button_bg_color,
    disabled_button_text_color,
    title_text_color, text_input_bg_color;


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
