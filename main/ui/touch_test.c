#include "esp_log.h"
#include "ui.h"
#include "ui_internal.h"


static lv_obj_t* screen = NULL;
static lv_obj_t* dot = NULL;


static void touch_test_event_handler(lv_event_t *e) {
    if (dot == NULL) {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t point;
    lv_indev_get_point(lv_indev_active(), &point);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_align(dot, LV_ALIGN_TOP_LEFT, point.x, point.y);
        ESP_LOGI("TOUCH_TEST", "Clicked at (%d, %d)", (int)point.x, (int)point.y);
    }
    if (code == LV_EVENT_PRESSING) {
        lv_obj_align(dot, LV_ALIGN_TOP_LEFT, point.x, point.y);
        //ESP_LOGI("TOUCH_TEST", "Pressed at (%d, %d)", (int)point.x, (int)point.y);
    }
    if (code == LV_EVENT_RELEASED) {
        //ESP_LOGI("TOUCH_TEST", "Released at (%d, %d)", (int)point.x, (int)point.y);
    }
}

void init_touch_test_ui() {
    screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(screen, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_add_event_cb(screen, touch_test_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    dot = lv_obj_create(screen);
    lv_obj_set_size(dot, 4, 4);
    lv_obj_set_style_bg_color(dot, lv_color_white(), 0);
    lv_obj_set_style_radius(dot, 5, 0);
    

}

