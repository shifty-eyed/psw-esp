#include "esp_log.h"

#include "ui.h"
#include "ui_styles.h"
#include "ui_internal.h"

#include "item_registry.h"

static const char *TAG = "UI";

typedef struct {
    lv_obj_t *tab,
            *list,
            *toolbar_button[3];
    int selected_item;
} tab_components_t;

static tab_components_t device_tab;
static tab_components_t password_tab;

static lv_obj_t* tabview;
//static lv_obj_t* loading_overlay;
static ui_api_callbacks_t* api_callbacks;
static lv_obj_t *toast = NULL;
static lv_timer_t *timer = NULL;


static void updale_list_items(lv_obj_t *list, registry_api_t *registry, lv_event_cb_t cb, char *symbol);

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

static bool select_list_item(tab_components_t* tab_data, int index) {
    lv_obj_t * obj_to_select = lv_obj_get_child(tab_data->list, index);
    if (obj_to_select == NULL || tab_data->selected_item == index) {
        return false;
    }
    if (tab_data->selected_item >= 0) {
        lv_obj_remove_state(lv_obj_get_child(tab_data->list, tab_data->selected_item), LV_STATE_CHECKED);
    }
    lv_obj_add_state(obj_to_select, LV_STATE_CHECKED);
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

static void device_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &device_tab, &device_registry_common)) {
        ESP_LOGI(TAG, "devices_tab.selected_item %d", device_tab.selected_item);
        api_callbacks->connect_to_device(device_tab.selected_item);
    }
}

static void password_list_item_cb(lv_event_t *e) {
    if (list_item_cb(e, &password_tab, &password_registry_common)) {
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
    pair_device_dialog_show();
    api_callbacks->pairing_initiated();
}

static void device_delete_cb(lv_event_t *e) {
    ESP_LOGI(TAG, "Click: Delete device");
    device_registry_common.remove(device_tab.selected_item);
    lv_obj_remove_state(lv_obj_get_child(device_tab.list, device_tab.selected_item), LV_STATE_CHECKED);
    device_tab.selected_item = -1;
    updale_list_items(device_tab.list, &device_registry_common, device_list_item_cb, LV_SYMBOL_BLUETOOTH);
    //lv_obj_t* item = lv_obj_get_child(device_tab.list, device_tab.selected_item);
    //lv_obj_delete(item);
}

static void device_disconnect_cb(lv_event_t *e) {
    lv_obj_remove_state(lv_obj_get_child(device_tab.list, device_tab.selected_item), LV_STATE_CHECKED);
    device_tab.selected_item = -1;
    evaluate_buttons_state();
    api_callbacks->disconnect();
    ESP_LOGI(TAG, "Click: Disconnect device");
}

static void updale_list_items(lv_obj_t *list, registry_api_t *registry, lv_event_cb_t cb, char *symbol) {
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

void ui_on_known_device_connected(int index) {
    ESP_LOGI(TAG, "ui_on_known_device_connected index=%d", index);
    show_toast("Device connected", false, 1000);
    select_list_item(&device_tab, index);
}

void ui_on_new_device_saved(int index) {
    show_toast("New Device connected", false, 1000);
    updale_list_items(device_tab.list, &device_registry_common, device_list_item_cb, LV_SYMBOL_BLUETOOTH);
    select_list_item(&device_tab, index);
}

void ui_on_new_device_paired() { //this is called while dialog has open
    pair_device_dialog_on_pairing_succeeded();
}

static lv_obj_t * create_floating_button(lv_obj_t *parent, char *symbol, int position, lv_event_cb_t cb) {
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_image_src(btn, symbol, 0);
    style_floating_button(btn, position);
    return btn;
}

static tab_components_t init_tab_view(char *text, lv_obj_t *parent, registry_api_t *registry, 
        lv_event_cb_t cb, int tab_id, char *symbol) {
    tab_components_t result;
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

static lv_obj_t * create_loading_overlay() {
    lv_obj_t * overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(overlay, SCREEN_W, SCREEN_H);
    lv_obj_t *spinner = lv_spinner_create(overlay);
    style_loading_overlay(overlay, spinner);
    return overlay;
}

static void toast_timer_cb(lv_timer_t *t) {
    lv_obj_delete(toast);
    lv_timer_delete(timer);
    toast = NULL;
    timer = NULL;
}

void show_toast(const char *message, bool is_error, uint32_t duration_ms) {
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

    if (timer) lv_timer_delete(timer);
    timer = lv_timer_create_basic();
    lv_timer_set_period(timer, duration_ms);
    lv_timer_set_repeat_count(timer, 1);
    lv_timer_set_user_data(timer, toast);
    lv_timer_set_cb(timer, toast_timer_cb);
}


void init_ui(ui_api_callbacks_t *callbacks) {
    ESP_LOGI(TAG, "init_ui()");

    api_callbacks = callbacks;

    tabview = lv_tabview_create(lv_screen_active());
    style_tabview(tabview);

    lv_obj_t * tab_buttons = lv_tabview_get_tab_bar(tabview);
    style_tab_buttons(tab_buttons);

    lv_obj_add_event_cb(tabview, tab_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    device_tab = init_tab_view("Devices", tabview, &device_registry_common, 
        device_list_item_cb, TAB_ID_DEVICE, LV_SYMBOL_BLUETOOTH);
    password_tab = init_tab_view("Passwords", tabview, &password_registry_common, 
        password_list_item_cb, TAB_ID_PASSWORD, LV_SYMBOL_KEYBOARD);

    evaluate_buttons_state();

    pair_device_dialog_init(callbacks);

    //loading_overlay = create_loading_overlay();
    //lv_obj_add_flag(loading_overlay, LV_OBJ_FLAG_HIDDEN);
}
