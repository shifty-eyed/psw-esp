#include "ui.h"
#include "ui_styles.h"
#include "esp_log.h"
//#include "../infra.h"
#include "item_registry.h"

static const char *TAG = "MAIN";

static tab_components device_tab;
static tab_components password_tab;

static lv_obj_t* tabview;

static void evaluate_buttons_state() {
    //int selected_tab_id = lv_tabview_get_tab_active(tabview);
    lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tabview);
    lv_obj_t * password_tab_btn = lv_obj_get_child(tab_bar, 1);
    lv_obj_set_style_text_color(password_tab_btn, tab_button_text_color_disabled, LV_STATE_DISABLED);

    if (device_tab.selected_item >= 0) {
        lv_obj_remove_state(password_tab_btn, LV_STATE_DISABLED);
        lv_obj_remove_state(device_tab.toolbar_button[BTN_DEVICE_DELETE], LV_STATE_DISABLED);
        lv_obj_remove_state(device_tab.toolbar_button[BTN_DEVICE_DISCONNECT], LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(password_tab_btn, LV_STATE_DISABLED);
        lv_obj_add_state(device_tab.toolbar_button[BTN_DEVICE_DELETE], LV_STATE_DISABLED);
        lv_obj_add_state(device_tab.toolbar_button[BTN_DEVICE_DISCONNECT], LV_STATE_DISABLED);
    }

    if (password_tab.selected_item >= 0) {
        lv_obj_remove_state(password_tab.toolbar_button[BTN_PASSWORD_EDIT], LV_STATE_DISABLED);
        lv_obj_remove_state(password_tab.toolbar_button[BTN_PASSWORD_DELETE], LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(password_tab.toolbar_button[BTN_PASSWORD_EDIT], LV_STATE_DISABLED);
        lv_obj_add_state(password_tab.toolbar_button[BTN_PASSWORD_DELETE], LV_STATE_DISABLED);
    }
}

static bool list_item_cb(lv_event_t *e, tab_components* tab_data, registry_api *registry) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * selected_obj = lv_event_get_target(e);
    int index = lv_obj_get_index(selected_obj);
    if (index >= registry->get_count()) {
        return false;
    }

    if (code == LV_EVENT_CLICKED) {
        lv_obj_remove_state(lv_obj_get_child(tab_data->list, tab_data->selected_item), LV_STATE_CHECKED);
        lv_obj_add_state(selected_obj, LV_STATE_CHECKED);
        tab_data->selected_item = index;
        evaluate_buttons_state();
        return true;
    }
    return false;
}

static void device_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &device_tab, &device_registry)) {
        ESP_LOGI(TAG, "devices_tab.selected_item %d", device_tab.selected_item);
    }
}

static void password_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &password_tab, &password_registry)) {
        ESP_LOGI(TAG, "passwords_tab.selected_item %d", password_tab.selected_item);
    }
}

static void password_add_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Add password");
}

static void password_use_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Use password");
}

static void password_edit_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Edit password");
}

static void device_add_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Add device");
}

static void device_delete_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete device");
}

static void device_disconnect_cb(lv_event_t *e) {
    lv_obj_remove_state(lv_obj_get_child(device_tab.list, device_tab.selected_item), LV_STATE_CHECKED);
    device_tab.selected_item = -1;
    evaluate_buttons_state();
    ESP_LOGI(TAG, "Click: Disconnect device");
}

static void updale_list_items(lv_obj_t *list, registry_api *registry, lv_event_cb_t cb, char *symbol) {
    int trailing_dummies = 2;
    lv_obj_clean(list);
    for (int i = 0; i < registry->get_count() + trailing_dummies; i++) {
        bool is_dummy = i >= registry->get_count();
        lv_obj_t *btn = lv_list_add_button(list, 
            is_dummy ? NULL : symbol, 
            is_dummy ? "" : registry->get_name(i));
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
        style_list_item(btn, i);
    }
}

static lv_obj_t * create_floating_button(lv_obj_t *parent, char *symbol, int position, lv_event_cb_t cb) {
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(btn, symbol, 0);
    style_floating_button(btn, position);
    return btn;
}

static tab_components init_tab_view(char *text, lv_obj_t *parent, registry_api *registry, 
        lv_event_cb_t cb, int tab_id, char *symbol) {
    tab_components result;
    result.selected_item = -1;
    result.tab = lv_tabview_add_tab(parent, text);
    style_tab_content(result.tab);

    result.list = lv_list_create(result.tab);
    style_tab_list(result.list);
    updale_list_items(result.list, registry, cb, symbol);

    result.toolbar_button[0] = create_floating_button(result.tab, LV_SYMBOL_PLUS, 0, tab_id == TAB_ID_DEVICE ? device_add_cb : password_add_cb);
    if (tab_id == TAB_ID_DEVICE) {
        result.toolbar_button[1] = create_floating_button(result.tab, LV_SYMBOL_POWER, 1, device_disconnect_cb);
        result.toolbar_button[2] = create_floating_button(result.tab, LV_SYMBOL_TRASH, 2, device_delete_cb);
    } else {
        result.toolbar_button[1] = create_floating_button(result.tab, LV_SYMBOL_EDIT, 1, password_edit_cb);
        result.toolbar_button[2] = create_floating_button(result.tab, LV_SYMBOL_OK, 2, password_use_cb);
    }

    return result;
}

void tab_switch_event_cb(lv_event_t * e) {
    lv_obj_remove_state(lv_obj_get_child(password_tab.list, password_tab.selected_item), LV_STATE_CHECKED);
    password_tab.selected_item = -1;
    evaluate_buttons_state();
}


void init_ui() {
    ESP_LOGI(TAG, "init_ui()");

    tabview = lv_tabview_create(lv_screen_active());
    style_tabview(tabview);

    lv_obj_t * tab_buttons = lv_tabview_get_tab_bar(tabview);
    style_tab_buttons(tab_buttons);

    lv_obj_add_event_cb(tabview, tab_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    device_tab = init_tab_view("Devices", tabview, &device_registry, 
        device_list_item_cb, TAB_ID_DEVICE, LV_SYMBOL_BLUETOOTH);
    password_tab = init_tab_view("Passwords", tabview, &password_registry, 
        password_list_item_cb, TAB_ID_PASSWORD, LV_SYMBOL_KEYBOARD);

    evaluate_buttons_state();
}
