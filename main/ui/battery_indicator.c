#include "ui.h"
#include "ui_internal.h"
#include "system/power_control.h"

static lv_timer_t *battery_state_timer = NULL;
static void battery_state_update(lv_timer_t *t);

void init_battery_indicator() {
    lv_obj_t *battery_state = lv_label_create(lv_screen_active());
    lv_obj_set_size(battery_state, 70, 30);
    lv_obj_align(battery_state, LV_ALIGN_TOP_MID, 5, 5);
    lv_obj_set_style_text_font(battery_state, lv_theme_get_font_small(battery_state), 0);
    lv_label_set_text(battery_state, "");
    battery_state_timer = lv_timer_create(battery_state_update, 5000, battery_state);
}

static void battery_state_update(lv_timer_t *t) {
    lv_obj_t *battery_state = (lv_obj_t *)t->user_data;
    float volts;
    int data;
    char buf[20];
    char symbol[8];
    adc_get_value(&volts, &data);

    if (volts > 4.2) {
        lv_obj_set_style_text_color(battery_state, lv_palette_main(LV_PALETTE_CYAN), 0);
    } else if (volts < 3.2) {
        lv_obj_set_style_text_color(battery_state, lv_palette_main(LV_PALETTE_RED), 0);
    } else {
        lv_obj_set_style_text_color(battery_state, lv_palette_main(LV_PALETTE_AMBER), 0);
    }
    
    if (volts > 4.2) {
        strcpy(symbol, LV_SYMBOL_CHARGE);
    } else if (volts > 3.9) {
        strcpy(symbol, LV_SYMBOL_BATTERY_FULL);
    } else if (volts > 3.7) {
        strcpy(symbol, LV_SYMBOL_BATTERY_3);
    } else if (volts > 3.45) {
        strcpy(symbol, LV_SYMBOL_BATTERY_2);
    } else if (volts > 3.2) {
        strcpy(symbol, LV_SYMBOL_BATTERY_1);
    } else {
        strcpy(symbol, LV_SYMBOL_BATTERY_EMPTY);
    }

    sprintf(buf, "%s %0.2fv", symbol, volts);
    lv_label_set_text(battery_state, buf);
}