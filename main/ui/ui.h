#ifndef UI_H
#define UI_H

#include "lvgl.h"

#define TAB_ID_DEVICE (0)
#define TAB_ID_PASSWORD (1)

#define SCREEN_W (240)
#define SCREEN_H (280)


#define TAB_BAR_H (30)
#define TOOL_BAR_H (30)

#define BTN_DEVICE_DELETE (3)
#define BTN_DEVICE_DISCONNECT (2)
#define BTN_DEVICE_CONNECT (1)
#define BTN_PASSWORD_EDIT (1)
#define BTN_PASSWORD_APPLY (2)

#define TOAST_DURATION (2000)

void init_ui();

void ui_on_known_device_connected(int index);
void ui_on_new_device_saved(int index);
void ui_on_new_device_paired();

void ui_on_password_dialog_closed(int index);

void init_touch_test_ui();



#endif // UI_H
