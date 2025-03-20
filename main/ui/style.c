#include "style.h"


lv_color_t row_bg_color1, row_bg_color2, row_bg_selected, row_text_color, delete_button_bg_color, title_text_color, text_input_bg_color;


void init_theme() {
    row_bg_color1 = lv_color_make(10, 10, 15);
    row_bg_color2 = lv_color_make(15, 15, 20);
    row_bg_selected = lv_color_make(40, 70, 200);
    row_text_color = lv_color_make(240, 240, 240);
    delete_button_bg_color = lv_color_make(180, 40, 40);
    title_text_color = lv_color_make(255, 255, 255);
    text_input_bg_color = lv_color_make(60, 60, 30);

    lv_theme_t *theme = lv_theme_default_init(
        lv_disp_get_default(),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,  // Dark mode
        LV_FONT_DEFAULT);
    lv_disp_set_theme(lv_disp_get_default(), theme);
}