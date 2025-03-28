#ifndef STYLE_H
#define STYLE_H

#include "lvgl.h"

void init_theme();

extern 
lv_color_t row_bg_color1, row_bg_color2, row_bg_selected, row_text_color, 
    delete_button_bg_color,
    cancel_button_bg_color,
    disabled_button_bg_color,
    disabled_button_text_color,
    title_text_color, text_input_bg_color;

#endif // STYLE_H