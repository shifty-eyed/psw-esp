#ifndef UI_H
#define UI_H

#include "lvgl.h"

#define TAB_ID_DEVICE (0)
#define TAB_ID_PASSWORD (1)

#define SCREEN_W (240)
#define SCREEN_H (280)


#define TAB_BAR_H (30)
#define TOOL_BAR_H (30)

#define BTN_DEVICE_DELETE (1)
#define BTN_DEVICE_DISCONNECT (2)
#define BTN_PASSWORD_EDIT (1)
#define BTN_PASSWORD_DELETE (2)

typedef struct {
    lv_obj_t *tab,
            *list,
            *toolbar_button[3];
    int selected_item;
} tab_components;

void init_ui();



#endif // UI_H
