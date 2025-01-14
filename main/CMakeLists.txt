set(srcs "main.c"
         "esp_hid_gap.c"
         "lcd_touch.c"
         "device_registry.c"
         "password_registry.c"
         "ui.c"
         )
set(include_dirs ".")

idf_component_register(SRCS "esp_hid_device_main.c" "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES esp_hid
                       PRIV_REQUIRES nvs_flash)
