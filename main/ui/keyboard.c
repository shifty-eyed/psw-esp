#include "keyboard.h"
#include "ui_internal.h"

static const char * device_name_kb_map_uc[] = 
   {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "\n",
    "-", "Z", "X", "C", "V", "B", "N", "M", ".", "\n",
    "1#", "abc", " ", LV_SYMBOL_BACKSPACE, ""
};

static const char * device_name_kb_map_lc[] = 
   {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
    "-", "z", "x", "c", "v", "b", "n", "m", ".", "\n",
    "1#", "ABC", " ", LV_SYMBOL_BACKSPACE, ""
};

static const lv_buttonmatrix_ctrl_t device_name_kb_ctrl_uc_and_lc_map[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4,
    LV_BUTTONMATRIX_CTRL_CHECKED | 1, 1, 1, 1, 1, 1, 1, 1, LV_BUTTONMATRIX_CTRL_CHECKED | 1,
    LV_KEYBOARD_CTRL_BUTTON_FLAGS | 5, LV_KEYBOARD_CTRL_BUTTON_FLAGS | 6, 6, LV_BUTTONMATRIX_CTRL_CHECKED | 5, 
};

static const char * device_name_kb_map_spec[] = 
   {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "+", "&", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
    "\\",  "@", "$", "(", ")", "{", "}", "[", "]", ";", "'", "\n",
    "1#", "ABC", " ", LV_SYMBOL_BACKSPACE, ""
};

static const lv_buttonmatrix_ctrl_t device_name_kb_ctrl_spec_map[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    LV_KEYBOARD_CTRL_BUTTON_FLAGS | 5, LV_KEYBOARD_CTRL_BUTTON_FLAGS | 5, 6, LV_BUTTONMATRIX_CTRL_CHECKED | 5, 
};


lv_obj_t * lv_my_keyboard_create(lv_obj_t * parent) {
    lv_obj_t * kb = lv_keyboard_create(parent);
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_SCROLLABLE);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, device_name_kb_map_uc, device_name_kb_ctrl_uc_and_lc_map);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, device_name_kb_map_lc, device_name_kb_ctrl_uc_and_lc_map);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_SPECIAL, device_name_kb_map_spec, device_name_kb_ctrl_spec_map);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);

    return kb;
}