#ifndef LVGL9TO8_H
#define LVGL9TO8_H

#ifdef LVGL8

#include "lvgl.h"

#define lv_indev_active lv_indev_get_act
#define lv_display_get_default lv_disp_get_default
#define lv_display_set_theme lv_disp_set_theme
#define lv_obj_remove_flag lv_obj_clear_flag
#define lv_obj_remove_state lv_obj_clear_state
#define lv_screen_active lv_scr_act
#define lv_timer_delete lv_timer_del
#define lv_list_add_button lv_list_add_btn
#define lv_button_create lv_btn_create
#define lv_obj_set_style_bg_image_src lv_obj_set_style_bg_img_src
#define lv_obj_delete lv_obj_del
#define lv_buttonmatrix_ctrl_t lv_btnmatrix_ctrl_t
#define LV_BUTTONMATRIX_CTRL_CHECKED LV_BTNMATRIX_CTRL_CHECKED
#define LV_KEYBOARD_CTRL_BUTTON_FLAGS LV_KEYBOARD_CTRL_BTN_FLAGS


void lv_obj_set_style_margin_all(struct _lv_obj_t * obj, lv_coord_t value, lv_style_selector_t selector);

#endif // LVGL8

#endif // LVGL9TO8_H