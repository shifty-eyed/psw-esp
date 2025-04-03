#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "driver/gpio.h"

#define PWR_BUTTON_GPIO GPIO_NUM_0

void init_gpio_and_power_task();
void adc_get_value(float *value,int *data);


#endif