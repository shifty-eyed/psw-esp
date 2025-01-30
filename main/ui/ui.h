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
#define BTN_DEVICE_CONNECT (3)
#define BTN_PASSWORD_EDIT (1)
#define BTN_PASSWORD_DELETE (2)

typedef struct {
    void (*pairing_initiated)();
    void (*disconnect)();
    void (*connect_to_device)(int i);
    void (*cancel_pairing)();
    void (*save_new_device)(const char *name);
    void (*delete_device)(int i);
} ui_api_callbacks_t;

void init_ui(ui_api_callbacks_t *callbacks);

void ui_on_known_device_connected(int index);
void ui_on_new_device_saved(int index);
void ui_on_new_device_paired();



#endif // UI_H
