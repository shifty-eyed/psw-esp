set(srcs "main.c"
         "system/lcd_touch.c"
         "system/power_control.c"
         "bt/esp_hidd_prf_api.c"
         "bt/hid_dev.c"
         "bt/hid_device_le_prf.c"        
         "bt/my_bt.c"
         "registry/device_registry.c"
         "registry/password_registry.c"
         "registry/my_nvs.c"
         "ui/ui.c"
         "ui/pair_device_dialog.c"
         "ui/edit_password_dialog.c"
         "ui/keyboard.c"
        )
set(include_dirs ".;./bt;./registry;./system;./ui")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES esp_hid
                       PRIV_REQUIRES nvs_flash)
