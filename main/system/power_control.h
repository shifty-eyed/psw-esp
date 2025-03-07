#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "driver/gpio.h"

#define SYS_EN_GPIO GPIO_NUM_35
#define PWR_BUTTON_GPIO GPIO_NUM_36

void init_gpio_and_power_task();


#endif