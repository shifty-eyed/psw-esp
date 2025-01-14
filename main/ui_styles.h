#ifndef UI_STYLES_H
#define UI_STYLES_H

#include "lvgl.h"

const lv_color_t row_bg_color1 = { .red = 10, .green = 10, .blue = 15 };
const lv_color_t row_bg_color2 = { .red = 15, .green = 15, .blue = 20 };
const lv_color_t row_bg_selected = { .red = 40, .green = 70, .blue = 200 };
const lv_color_t row_text_color = { .red = 240, .green = 240, .blue = 240 };


const lv_color_t tab_button_bg_color = { .red = 40, .green = 40, .blue = 40 };
const lv_color_t tab_button_text_color = { .red = 255, .green = 255, .blue = 255 };
const lv_color_t tab_button_text_color_disabled = { .red = 110, .green = 60, .blue = 60 };


static void style_list_item(lv_obj_t *btn, int i) {
    lv_obj_set_style_bg_color(btn, (i % 2) ? row_bg_color1 : row_bg_color2, 0);
    lv_obj_set_style_bg_color(btn, row_bg_selected, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(btn, row_text_color, 0);
    lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
}

static void style_tab_content(lv_obj_t *tab) {
    lv_obj_remove_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_all(tab, 0, 0);
    lv_obj_set_style_pad_all(tab, 0, 0);
    lv_obj_set_style_bg_opa(tab, LV_OPA_0, 0);
}

static void style_tab_list(lv_obj_t *list) {
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_width(list, SCREEN_W-5, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_0, 0);
    lv_obj_set_style_margin_all(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_border_side(list, LV_BORDER_SIDE_NONE, 0);
}

static void style_tabview(lv_obj_t *tabview) {
    lv_tabview_set_tab_bar_size(tabview, TAB_BAR_H);
    lv_obj_set_style_bg_color(tabview, lv_color_black(), 0);
}

static void style_tab_buttons(lv_obj_t *tab_buttons) {
    lv_obj_set_style_bg_color(tab_buttons, tab_button_bg_color, 0);
    lv_obj_set_style_text_color(tab_buttons, tab_button_text_color, 0);
    lv_obj_set_style_border_side(tab_buttons, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);
}

static void style_floating_button(lv_obj_t *btn, int position) {
    int gap = 55;
    int offset = 20;
    lv_obj_set_size(btn, 50, 50);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -offset - (position * gap), -10);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_text_font(btn, lv_theme_get_font_large(btn), 0);
}

#endif
