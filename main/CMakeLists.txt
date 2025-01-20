set(srcs "main.c"
         "bt/esp_hidd_prf_api.c"
         "bt/hid_dev.c"
         "bt/hid_device_le_prf.c"        
         "lcd_touch.c"
         "device_registry.c"
         "password_registry.c"
         "ui.c"
         "my_bt.c"
        )
set(include_dirs ".;./bt")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES esp_hid
                       PRIV_REQUIRES nvs_flash)
