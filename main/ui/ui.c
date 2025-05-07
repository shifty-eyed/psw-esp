#include "esp_log.h"

#include "ui.h"
#include "ui_internal.h"

#include "registry/item_registry.h"
#include "bt/my_bt.h"

static const char *TAG = "UI";

typedef struct {
    lv_obj_t *tab,
            *list,
            *menu,
            *toolbar_button[4];
    int selected_item;
} tab_components_t;

static tab_components_t device_tab;
static tab_components_t password_tab;

static lv_obj_t* tabview;
static lv_obj_t* spinner;
static lv_timer_t *device_connect_timer = NULL;

static lv_obj_t *toast = NULL;
static lv_timer_t *timer = NULL;

static void updale_list_items(lv_obj_t *list, registry_api_t *registry, lv_event_cb_t cb, char *symbol);

void ui_on_touch_pressed(int x, int y) {
    if (device_tab.menu && popup_menu_visible(device_tab.menu)) {
        delayed_menu_hide(device_tab.menu);
    }
    if (password_tab.menu && popup_menu_visible(password_tab.menu)) {
        delayed_menu_hide(password_tab.menu);
    }
}

static void evaluate_buttons_state() {
    bool connected = bt_is_connected();
    bool device_selected = device_tab.selected_item >= 0;
    bool password_selected = password_tab.selected_item >= 0;

    lv_show(device_tab.toolbar_button[BTN_DEVICE_DELETE], device_selected && !connected);
    lv_show(device_tab.toolbar_button[BTN_DEVICE_CONNECT], device_selected && !connected);
    lv_show(device_tab.toolbar_button[BTN_DEVICE_DISCONNECT], connected);

    lv_show(password_tab.toolbar_button[BTN_PASSWORD_EDIT], password_selected);
    lv_show(password_tab.toolbar_button[BTN_PASSWORD_APPLY], password_selected);
}

static void unselect_list_item(tab_components_t* tab_data) {
    if (tab_data->selected_item >= 0) {
        lv_obj_t* item = lv_obj_get_child(tab_data->list, tab_data->selected_item);
        if (item != NULL) {
            lv_obj_remove_state(item, LV_STATE_CHECKED);
        }
    }
    tab_data->selected_item = -1;
    evaluate_buttons_state();
}

static bool select_list_item(tab_components_t* tab_data, int index) {
    if (index < 0) {
        return false;
    }
    if (tab_data->selected_item >= 0) {
        lv_obj_t* item = lv_obj_get_child(tab_data->list, tab_data->selected_item);
        if (item != NULL) {
            lv_obj_remove_state(item, LV_STATE_CHECKED);
        }
    }
    lv_obj_t * obj_to_select = lv_obj_get_child(tab_data->list, index);
    if (obj_to_select != NULL) {
        lv_obj_add_state(obj_to_select, LV_STATE_CHECKED);
    }
    tab_data->selected_item = index;
    evaluate_buttons_state();
    return true;
}

static bool list_item_cb(lv_event_t *e, tab_components_t* tab_data, registry_api_t *registry) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * selected_obj = lv_event_get_target(e);
    int index = lv_obj_get_index(selected_obj);

    if (code == LV_EVENT_CLICKED && index >= 0 && index < registry->get_count()) {
        return select_list_item(tab_data, index);
    }
    return false;
}

static void device_connection_timeout_cb(lv_timer_t *t) {
    bt_stop_advertising();
    lv_timer_delete(device_connect_timer);
    device_connect_timer = NULL;
    show_spinner(false);
    show_toast("Connection time out", true);
    unselect_list_item(&device_tab);
}

static void device_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &device_tab, &device_registry_common)) {
        ESP_LOGI(TAG, "devices_tab.selected_item %d", device_tab.selected_item);
        evaluate_buttons_state();
    }
}

static void password_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &password_tab, &password_registry_common)) {
        ESP_LOGI(TAG, "passwords_tab.selected_item %d", password_tab.selected_item);
    }
}

static void password_use_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Use password");
    password_entry_t *entry = password_registry_get_entry_by_index(password_tab.selected_item);
    bt_hid_send_keyboard_string_sequence(entry->password);
}

static void password_add_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: New password");
    edit_password_dialog_show(NULL);
}

static void password_edit_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Edit password");
    edit_password_dialog_show(password_registry_get_entry_by_index(password_tab.selected_item));
}

static void device_add_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Add device");
    pair_device_dialog_show();
    bt_start_advertising();
}

static void device_delete_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete device");
    device_registry_remove_device(device_tab.selected_item);
    lv_obj_remove_state(lv_obj_get_child(device_tab.list, device_tab.selected_item), LV_STATE_CHECKED);
    device_tab.selected_item = -1;
    updale_list_items(device_tab.list, &device_registry_common, device_list_item_cb, LV_SYMBOL_BLUETOOTH);
}

static void device_disconnect_cb(lv_event_t *e) {
    bt_disconnect(current_device.addr);
    unselect_list_item(&device_tab);
    show_toast("Device disconnected", false);
    ESP_LOGI(TAG, "Click: Disconnect device");
}

static void device_connect_cb(lv_event_t *e) {
    device_entry_t *device = device_registry_get_by_index(device_tab.selected_item);
    if (bt_direct_advertizing(device->addr, device->addr_type)) {
        show_spinner(true);
        if (device_connect_timer) {
            lv_timer_delete(device_connect_timer);
        }
        device_connect_timer = lv_timer_create(device_connection_timeout_cb, 5000, NULL);
    } else {
        unselect_list_item(&device_tab);
    }
}

static void updale_list_items(lv_obj_t *list, registry_api_t *registry, lv_event_cb_t cb, char *symbol) {
    lv_obj_clean(list);
    for (int i = 0; i < registry->get_count(); i++) {
        lv_obj_t *btn = lv_list_add_button(list, symbol, registry->get_name(i));
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(btn, (i % 2) ? row_bg_color1 : row_bg_color2, 0);
        lv_obj_set_style_bg_color(btn, row_bg_selected, LV_STATE_CHECKED);
        lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_height(btn, LIST_ITEM_H);
    }
}

void ui_on_known_device_connected(int index) {
    ESP_LOGI(TAG, "ui_on_known_device_connected index=%d", index);
    lv_timer_delete(device_connect_timer);
    device_connect_timer = NULL;
    show_spinner(false);

    show_toast("Device connected", false);
    evaluate_buttons_state();
}

void ui_on_new_device_saved(int index) {
    show_toast("New Device connected", false);
    updale_list_items(device_tab.list, &device_registry_common, device_list_item_cb, LV_SYMBOL_BLUETOOTH);
    select_list_item(&device_tab, index);
}

void ui_on_new_device_paired() { //this is called while dialog has open
    pair_device_dialog_on_pairing_succeeded();
}

void ui_on_password_dialog_closed(int index) {
    show_toast(index >= 0 ? "Password saved" : "Password deleted", false);
    updale_list_items(password_tab.list, &password_registry_common, password_list_item_cb, LV_SYMBOL_KEYBOARD);
    select_list_item(&password_tab, index);
}

static tab_components_t init_tab_view(char *text, lv_obj_t *parent, registry_api_t *registry, 
        lv_event_cb_t cb, int tab_id, char *symbol) {
    tab_components_t result;
    result.selected_item = -1;
    result.tab = lv_tabview_add_tab(parent, text);
    lv_obj_remove_flag(result.tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_all(result.tab, 0, 0);
    lv_obj_set_style_pad_all(result.tab, 0, 0);
    lv_obj_set_style_border_side(result.tab, LV_BORDER_SIDE_FULL, 0);

    result.list = lv_list_create(result.tab);
    lv_obj_align(result.list, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_width(result.list, SCREEN_W-5, 0);
    lv_obj_set_style_margin_all(result.list, 0, 0);
    lv_obj_set_style_pad_all(result.list, 0, 0);
    lv_obj_set_style_pad_ver(result.list, 5, 0);
    lv_obj_set_style_border_side(result.list, LV_BORDER_SIDE_NONE, 0);

    updale_list_items(result.list, registry, cb, symbol);

    if (tab_id == TAB_ID_DEVICE) {
        result.menu = popup_menu_create(result.tab, 4);
        result.toolbar_button[BTN_DEVICE_CONNECT] = popup_menu_add_button(result.menu, "Connect", LV_SYMBOL_POWER, device_connect_cb);
        result.toolbar_button[BTN_DEVICE_DISCONNECT] = popup_menu_add_button(result.menu, "Disconnect", LV_SYMBOL_EJECT, device_disconnect_cb);
        result.toolbar_button[BTN_DEVICE_DELETE] = popup_menu_add_button(result.menu, "Delete Device", LV_SYMBOL_TRASH, device_delete_cb);
        result.toolbar_button[BTN_ITEM_ADD] = popup_menu_add_button(result.menu, "Add Device", LV_SYMBOL_PLUS, device_add_cb);

    } else {
        result.menu = popup_menu_create(result.tab, 4);
        result.toolbar_button[BTN_PASSWORD_EDIT] = popup_menu_add_button(result.menu, "Edit Password", LV_SYMBOL_EDIT, password_edit_cb);
        result.toolbar_button[BTN_PASSWORD_DELETE] = popup_menu_add_button(result.menu, "Delete Password", LV_SYMBOL_EDIT, password_edit_cb);
        result.toolbar_button[BTN_PASSWORD_APPLY] = popup_menu_add_button(result.menu, "Use password", LV_SYMBOL_OK, password_use_cb);
        result.toolbar_button[BTN_ITEM_ADD] = popup_menu_add_button(result.menu, "Add Password", LV_SYMBOL_PLUS, password_add_cb);
    }

    return result;
}

void tab_switch_event_cb(lv_event_t * e) {
    lv_obj_t* selected_item = lv_obj_get_child(password_tab.list, password_tab.selected_item);
    if (selected_item) {
        lv_obj_remove_state(selected_item, LV_STATE_CHECKED);
    }
    password_tab.selected_item = -1;
    evaluate_buttons_state();
}

static void toast_timer_cb(lv_timer_t *t) {
    lv_obj_delete(toast);
    lv_timer_delete(timer);
    toast = NULL;
    timer = NULL;
}

void show_toast(const char *message, bool is_error) {
    if (toast) {
        return;
    }
    toast = lv_obj_create(lv_scr_act());
    lv_obj_set_size(toast, 200, 50);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_bg_color(toast, lv_palette_main(is_error ? LV_PALETTE_RED : LV_PALETTE_BLUE), 0);
    lv_obj_set_style_radius(toast, 10, 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_set_style_text_color(toast, lv_color_white(), 0);
    lv_obj_remove_flag(toast, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *label = lv_label_create(toast);
    lv_obj_center(label);

    lv_label_set_text(lv_obj_get_child(toast, 0), message);

    if (timer) lv_timer_del(timer);
    timer = lv_timer_create(toast_timer_cb, TOAST_DURATION, toast);
    lv_timer_set_repeat_count(timer, 1);
}

void show_spinner(bool show) {
    if (!spinner) {
        spinner = lv_spinner_create(lv_screen_active(), 1000, 60);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(spinner, SPINNER_SIZE, SPINNER_SIZE);
    }
    if (show) {
        lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    }
}

void init_ui() {
    ESP_LOGI(TAG, "init_ui()");
    init_color_theme();

    #ifdef LVGL8
    tabview = lv_tabview_create(lv_screen_active(), LV_DIR_TOP, TAB_BAR_H);
    lv_obj_t * tab_buttons = lv_tabview_get_tab_btns(tabview);
    
    #else
    tabview = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_size(tabview, TAB_BAR_H);
    lv_obj_t * tab_buttons = lv_tabview_get_tab_bar(tabview);
    #endif

    lv_obj_set_style_border_side(tab_buttons, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_add_event_cb(tabview, tab_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(tabview, SCREEN_W, SCREEN_H);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, ycoord(0));

    device_tab = init_tab_view("Devices", tabview, &device_registry_common, 
        device_list_item_cb, TAB_ID_DEVICE, LV_SYMBOL_BLUETOOTH);
    password_tab = init_tab_view("Passwords", tabview, &password_registry_common, 
        password_list_item_cb, TAB_ID_PASSWORD, LV_SYMBOL_KEYBOARD);

    evaluate_buttons_state();

    pair_device_dialog_init();
    edit_password_dialog_init();
    init_battery_indicator();

}
