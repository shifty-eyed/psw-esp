#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

void pair_device_dialog_init();
bool pair_device_dialog_is_open();
void pair_device_dialog_show();
void pair_device_dialog_on_pairing_succeeded();

void show_toast(const char *message, bool is_error, uint32_t duration_ms);

#endif // UI_INTERNAL_H