#ifndef UI_H
#define UI_H

#include "lvgl.h"

#define TAB_ID_DEVICE (0)
#define TAB_ID_PASSWORD (1)


#define LCD_H_RES (466)
#define LCD_V_RES (466)

#define SCREEN_W (360)
#define SCREEN_H (440)

#define TAB_BAR_H (50)
#define TOOL_BAR_H (50)
#define LIST_ITEM_H (50)

#define FLOATING_BUTTON_SIZE (80)
#define FLOATING_BUTTON_GAP (90)
#define FLOATING_BUTTON_OFFSET (-50)

#define SPINNER_SIZE (50)

#define BTN_ITEM_ADD (0)
#define BTN_DEVICE_CONNECT (1)
#define BTN_DEVICE_DISCONNECT (2)
#define BTN_DEVICE_DELETE (3)

#define BTN_PASSWORD_APPLY (1)
#define BTN_PASSWORD_EDIT (2)
#define BTN_PASSWORD_BACKUP (3)

#define TOAST_DURATION (2000)

#define COORD_RATIO_X (1.0)
#define COORD_RATIO_Y (1.0)

// edit password dialog
#define DIALOG_TOP_BAR_WIDTH (300)
#define DIALOG_WIDTH 400
#define DIALOG_HEIGHT 400

#define KEYBOARD_WIDTH DIALOG_WIDTH
#define KEYBOARD_HEIGHT (250)
#define KEYBOARD_OFFSET_V (15)


void init_ui();

void ui_on_known_device_connected(int index);
void ui_on_new_device_saved(int index);
void ui_on_new_device_paired();

void ui_on_password_dialog_closed(int index);
void ui_on_touch_pressed(int x, int y);

void init_touch_test_ui();



#endif // UI_H
