#ifndef LCD_TOUCH_H
#define LCD_TOUCH_H

#include "esp_lvgl_port.h"

void init_lcd_and_touch(void);
esp_err_t set_lcd_brightness(int brightness_percent);


/* LCD size */
#define BOARD_LCD_H_RES (240)
#define BOARD_LCD_V_RES (280)

/* LCD settings */
#define BOARD_LCD_SPI_NUM (SPI3_HOST)
#define BOARD_LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define BOARD_LCD_CMD_BITS (8)
#define BOARD_LCD_PARAM_BITS (8)
#define BOARD_LCD_COLOR_SPACE (ESP_LCD_COLOR_SPACE_RGB)
#define BOARD_LCD_BITS_PER_PIXEL (16)
#define BOARD_LCD_DRAW_BUFF_DOUBLE (1)
#define BOARD_LCD_DRAW_BUFF_HEIGHT (50)
#define BOARD_LCD_BL_ON_LEVEL (1)

/* LCD pins */
#define BOARD_LCD_GPIO_SCLK (GPIO_NUM_6)
#define BOARD_LCD_GPIO_MOSI (GPIO_NUM_7)
#define BOARD_LCD_GPIO_RST (GPIO_NUM_8)
#define BOARD_LCD_GPIO_DC (GPIO_NUM_4)
#define BOARD_LCD_GPIO_CS (GPIO_NUM_5)
#define BOARD_LCD_GPIO_BL (GPIO_NUM_15)

#define TOUCH_HOST I2C_NUM_0

#define BOARD_PIN_NUM_TOUCH_SCL (GPIO_NUM_10)
#define BOARD_PIN_NUM_TOUCH_SDA (GPIO_NUM_11)
#define BOARD_PIN_NUM_TOUCH_RST (GPIO_NUM_13)
#define BOARD_PIN_NUM_TOUCH_INT (GPIO_NUM_14)

#define LCD_LEDC_CH (1)

#endif  // LCD_TOUCH_H