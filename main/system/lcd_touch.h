#ifndef LCD_TOUCH_H
#define LCD_TOUCH_H

#include "esp_lcd_sh8601.h"

void init_lcd_and_touch(void);
void lcd_panel_on(bool value);

bool lvgl_port_lock(int timeout_ms);
void lvgl_port_unlock(void);


/* LCD size */
#define BOARD_LCD_H_RES (466)
#define BOARD_LCD_V_RES (466)

#define EXAMPLE_LCD_H_RES (BOARD_LCD_H_RES)
#define EXAMPLE_LCD_V_RES (BOARD_LCD_V_RES)

#define LCD_LEDC_CH (1)




#endif  // LCD_TOUCH_H