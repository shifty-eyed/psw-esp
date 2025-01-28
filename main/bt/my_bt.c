
#include "my_bt.h"

static const char *TAG = "MYBT";

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

static uint16_t hid_conn_id = 0;
static bool device_connected = false;
static volatile bool known_device_advertised = false;

static esp_bd_addr_t connected_device_address;
static esp_link_key connected_device_key;
static esp_ble_addr_type_t connected_device_address_type;

static bt_api_callbacks_t *api_callbacks;

#define HIDD_DEVICE_NAME            "HID Test"
static uint8_t hidd_service_uuid128[] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};

static esp_ble_adv_data_t hidd_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x03c0,       //HID Generic,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(hidd_service_uuid128),
    .p_service_uuid = hidd_service_uuid128,
    .flag = 0x6,
};

static esp_ble_adv_params_t hidd_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x30,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_params_t direct_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x30,
    .adv_type = ADV_TYPE_DIRECT_IND_HIGH,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST,
};

static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param) {
    switch(event) {
        case ESP_HIDD_EVENT_REG_FINISH: {
            if (param->init_finish.state == ESP_HIDD_INIT_OK) {
                //esp_bd_addr_t rand_addr = {0x04,0x11,0x11,0x11,0x11,0x05};
                esp_ble_gap_set_device_name(HIDD_DEVICE_NAME);
                esp_ble_gap_config_adv_data(&hidd_adv_data);

            }
            break;
        }
        case ESP_BAT_EVENT_REG: {
            break;
        }
        case ESP_HIDD_EVENT_DEINIT_FINISH:
	     break;
		case ESP_HIDD_EVENT_BLE_CONNECT: {
            ESP_LOGI(TAG, "ESP_HIDD_EVENT_BLE_CONNECT");
            hid_conn_id = param->connect.conn_id;
            break;
        }
        case ESP_HIDD_EVENT_BLE_DISCONNECT: {
            device_connected = false;
            ESP_LOGI(TAG, "ESP_HIDD_EVENT_BLE_DISCONNECT");
            //esp_ble_gap_start_advertising(&hidd_adv_params);
            break;
        }
        case ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT: {
            ESP_LOGI(TAG, "%s, ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT", __func__);
            ESP_LOG_BUFFER_HEX(TAG, param->vendor_write.data, param->vendor_write.length);
            break;
        }
        case ESP_HIDD_EVENT_BLE_LED_REPORT_WRITE_EVT: {
            ESP_LOGI(TAG, "ESP_HIDD_EVENT_BLE_LED_REPORT_WRITE_EVT");
            ESP_LOG_BUFFER_HEX(TAG, param->led_write.data, param->led_write.length);
            break;
        }
        default:
            break;
    }
    return;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG, "can esp_ble_gap_start_advertising");
        //esp_ble_gap_start_advertising(&hidd_adv_params);

        break;
     case ESP_GAP_BLE_SEC_REQ_EVT:
        for(int i = 0; i < ESP_BD_ADDR_LEN; i++) {
             ESP_LOGD(TAG, "%x:",param->ble_security.ble_req.bd_addr[i]);
        }
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
	 break;
     case ESP_GAP_BLE_AUTH_CMPL_EVT:
        device_connected = true;
        if(param->ble_security.auth_cmpl.success) {
            memcpy(connected_device_address, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            memcpy(connected_device_key, param->ble_security.auth_cmpl.key, sizeof(esp_link_key));
            connected_device_address_type = param->ble_security.auth_cmpl.addr_type;

            api_callbacks->on_device_connected(&(param->ble_security.auth_cmpl.bd_addr), 
                &(param->ble_security.auth_cmpl.addr_type), known_device_advertised);
            if (known_device_advertised) {
            } else {
                ESP_LOGI(TAG, "Connect completed! known=%d", known_device_advertised);
            }
        } else {
            ESP_LOGE(TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
        }


        ESP_LOGW(TAG, "connected to remote BD_ADDR: %08x%04x",\
                (connected_device_address[0] << 24) + (connected_device_address[1] << 16) 
                + (connected_device_address[2] << 8) + connected_device_address[3],
                (connected_device_address[4] << 8) + connected_device_address[5]);
        ESP_LOGI(TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
        break;
    default:
        ESP_LOGI(TAG, "event = %d", event);
        break;
    }
}

void bt_start_advertising() {
    if (device_connected) {
        ESP_LOGW(TAG, "Device already connected");
        return;
    }
    known_device_advertised = false;
    ESP_LOGW(TAG, "Starting advertising");
    esp_ble_gap_start_advertising(&hidd_adv_params);
}

void bt_disconnect() {
    device_connected = false;
    esp_ble_gap_disconnect(connected_device_address);
    // todo figure out if this is necessary
    //esp_ble_gap_update_whitelist(false, connected_device_address, (esp_ble_wl_addr_type_t)connected_device_address_type);
}

void bt_stop_advertising() {
    ESP_LOGW(TAG, "Stop advertising");
    esp_ble_gap_stop_advertising();
}

void bt_direct_advertizing(esp_bd_addr_t addr, esp_ble_addr_type_t addr_type) {
    if (device_connected) {
        ESP_LOGW(TAG, "Device already connected");
        return;
    }

    ESP_LOGW(TAG, "Starting direct advertising to %08x%04x", 
        (addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + addr[3],
        (addr[4] << 8) + addr[5]);
    known_device_advertised = true;
    esp_ble_gap_update_whitelist(true, addr, (esp_ble_wl_addr_type_t)addr_type);
    direct_adv_params.peer_addr_type = addr_type;
    memcpy(direct_adv_params.peer_addr, addr, sizeof(esp_bd_addr_t));
    esp_ble_gap_start_advertising(&direct_adv_params);
}


void init_bluetooth(bt_api_callbacks_t *callbacks) {

    api_callbacks = callbacks; 
    
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed", __func__);
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluedroid failed", __func__);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluedroid failed", __func__);
        return;
    }

    if((ret = esp_hidd_profile_init()) != ESP_OK) {
        ESP_LOGE(TAG, "%s init bluedroid failed", __func__);
    }

    ///register the callback function to the gap module
    esp_ble_gap_register_callback(gap_event_handler);
    esp_hidd_register_callbacks(hidd_event_callback);

    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}