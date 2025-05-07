#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_internal.h"

static const char *TAG = "MENU";

static lv_timer_t *menu_hide_timer = NULL;

#define MENU_WIDTH (260)
#define MENU_ITEM_HEIGHT (45)
#define MENU_ITEM_GAP (15)
#define FLAG_MENU_VISIBLE LV_OBJ_FLAG_USER_1

#define MENU_POS_INVISIBLE (345)

#define POPUP_MENU_BG_COLOR lv_palette_darken(LV_PALETTE_BLUE_GREY, 4)
#define POPUP_MENU_ICON_COLOR lv_palette_darken(LV_PALETTE_BLUE, 3)

static void start_animation_menu(lv_obj_t* menu, bool show);

bool popup_menu_visible(lv_obj_t* menu) {
    return lv_obj_has_flag(menu, FLAG_MENU_VISIBLE);
}

static void animate_y_cb(void * obj, int32_t v) {
    lv_obj_set_y((lv_obj_t *)obj, v);
}

static lv_coord_t menu_calc_height(lv_obj_t* menu) {
    int number_of_visible_items = 0;
    for (int i = 0; i < lv_obj_get_child_cnt(menu); i++) {
        lv_obj_t* item = lv_obj_get_child(menu, i);
        if (!lv_obj_has_flag(item, LV_OBJ_FLAG_HIDDEN)) {
            number_of_visible_items++;
        }
    }
    return ycoord(number_of_visible_items * (MENU_ITEM_HEIGHT + MENU_ITEM_GAP));
    
}

static void on_menu_button_click(lv_event_t *e) {
    lv_obj_t *menu = lv_obj_get_parent(lv_event_get_target(e));
    if (popup_menu_visible(menu)) {
        return;
    }

    lv_obj_t *parent = lv_obj_get_parent(menu);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_coord_t menu_h = menu_calc_height(menu) + ycoord(100);

    lv_obj_set_height(menu, menu_h);
    ESP_LOGI(TAG, "popup_menu_toggle_show: menu_h=%d", menu_h);
    lv_obj_add_flag(menu, FLAG_MENU_VISIBLE);
    start_animation_menu(menu, true);
}

static void menu_hide_timer_cb(lv_timer_t *t) {
    lv_timer_del(menu_hide_timer);
    menu_hide_timer = NULL;
    lv_obj_t *menu = t->user_data;
    lv_obj_t *parent = lv_obj_get_parent(menu);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    ESP_LOGI(TAG, "delayed_menu_hide_timer_cb");
    lv_obj_clear_flag(menu, FLAG_MENU_VISIBLE);
    start_animation_menu(menu, false);
}

void delayed_menu_hide(lv_obj_t* menu) {
    if (menu_hide_timer) {
        lv_timer_del(menu_hide_timer);
    }
    menu_hide_timer = lv_timer_create(menu_hide_timer_cb, 200, menu);
    lv_timer_set_repeat_count(menu_hide_timer, 1);
    ESP_LOGI(TAG, "delayed_menu_hide starting: lv_obj_get_y=%d", lv_obj_get_y(menu));
}

static void start_animation_menu(lv_obj_t* menu, bool show) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, menu);
    lv_anim_set_exec_cb(&a, animate_y_cb);
    lv_anim_set_time(&a, 200);
    lv_coord_t pos_visible = MENU_POS_INVISIBLE - menu_calc_height(menu);
    if (show) {
        lv_anim_set_values(&a, MENU_POS_INVISIBLE, pos_visible);
        ESP_LOGI(TAG, "start_animation_menu: (show) from %d to %d", MENU_POS_INVISIBLE, pos_visible);
    } else {
        lv_anim_set_values(&a, pos_visible, MENU_POS_INVISIBLE);
        ESP_LOGI(TAG, "start_animation_menu: (hide) from %d to %d", pos_visible, MENU_POS_INVISIBLE);
    }
    lv_anim_start(&a);
}

lv_obj_t* popup_menu_create(lv_obj_t* parent, int num_items) {

    lv_obj_t* menu = mylv_create_container_flex(parent, LV_FLEX_FLOW_COLUMN, 
        xcoord(MENU_WIDTH), ycoord(num_items * (MENU_ITEM_HEIGHT + MENU_ITEM_GAP)));
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(menu, LV_OBJ_FLAG_FLOATING);
    lv_obj_clear_flag(menu, FLAG_MENU_VISIBLE);
    lv_obj_align(menu, LV_ALIGN_TOP_MID, 0, MENU_POS_INVISIBLE);
    lv_obj_set_style_bg_opa(menu, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(menu, POPUP_MENU_BG_COLOR, 0);

    lv_obj_set_style_border_side(menu, LV_BORDER_SIDE_FULL, 0);
    lv_obj_set_style_border_width(menu, 2, 0);
    lv_obj_set_style_border_color(menu, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);


    lv_obj_t* menu_toggle_button = lv_button_create(menu);
    lv_obj_add_event_cb(menu_toggle_button, on_menu_button_click, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(menu_toggle_button, xcoord(MENU_WIDTH-10), xcoord(45));
    
    lv_obj_set_style_bg_color(menu_toggle_button, lv_palette_darken(LV_PALETTE_BLUE_GREY, 4), 0);
    lv_obj_set_style_bg_image_src(menu_toggle_button, LV_SYMBOL_LIST, 0);
    lv_obj_set_style_border_side(menu_toggle_button, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(menu_toggle_button, 1, 0);
    lv_obj_set_style_border_color(menu_toggle_button, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);

    return menu;
}

lv_obj_t* popup_menu_add_button(lv_obj_t* menu, const char* text,  const char* symbol, lv_event_cb_t cb) {
    lv_obj_t* container = mylv_create_container_flex(menu, LV_FLEX_FLOW_ROW, 
        xcoord(MENU_WIDTH), ycoord(MENU_ITEM_HEIGHT + MENU_ITEM_GAP));
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_hor(container, xcoord(10), 0);

    lv_obj_t* icon_btn = lv_button_create(container);
    lv_obj_set_size(icon_btn, xcoord(MENU_ITEM_HEIGHT), ycoord(MENU_ITEM_HEIGHT));
    lv_obj_add_event_cb(icon_btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(icon_btn, POPUP_MENU_ICON_COLOR, 0);
    lv_obj_set_style_bg_image_src(icon_btn, symbol, 0);

    lv_obj_t* caption_btn = lv_button_create(container);
    lv_obj_set_size(caption_btn, xcoord(180), ycoord(MENU_ITEM_HEIGHT));
    lv_obj_add_event_cb(caption_btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(caption_btn, POPUP_MENU_BG_COLOR, 0);
    lv_obj_t*label = lv_label_create(caption_btn);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, ycoord(0));
    
    return container;
}

