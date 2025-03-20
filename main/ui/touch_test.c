#include "esp_log.h"
#include "ui.h"
#include "ui_internal.h"
#include "lvgl9to8.h"
#include "lcd_touch.h"


static lv_obj_t* screen = NULL;
static lv_obj_t* dot = NULL;

static int dotsize = 40;

static void touch_test_event_handler(lv_event_t *e) {
    if (dot == NULL) {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t point;
    lv_indev_get_point(lv_indev_active(), &point);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_align(dot, LV_ALIGN_DEFAULT, point.x - dotsize/2, point.y - dotsize/2);
        ESP_LOGI("TOUCH_TEST", "Clicked at (%d, %d)", (int)point.x, (int)point.y);
    }
    if (code == LV_EVENT_PRESSING) {
        //ESP_LOGI("TOUCH_TEST", "Pressed at (%d, %d)", (int)point.x, (int)point.y);
    }
    if (code == LV_EVENT_RELEASED) {
        //ESP_LOGI("TOUCH_TEST", "Released at (%d, %d)", (int)point.x, (int)point.y);
    }
}

void init_touch_test_ui() {
    lv_color_t row_bg_color1 = lv_color_make(0, 200, 200);

    lv_theme_t *theme = lv_theme_default_init(
        lv_display_get_default(),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,  // Dark mode
        LV_FONT_DEFAULT);
    lv_display_set_theme(lv_display_get_default(), theme);

    screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(screen, SCREEN_W, SCREEN_H);
    lv_obj_add_event_cb(screen, touch_test_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    dot = lv_obj_create(screen);
    lv_obj_set_size(dot, dotsize, dotsize);
    lv_obj_set_style_bg_color(dot, row_bg_color1, 0);
    lv_obj_set_style_radius(dot, dotsize/4, 0);
    

}

