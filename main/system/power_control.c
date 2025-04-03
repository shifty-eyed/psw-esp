#include "power_control.h"
#include "freertos/FreeRTOS.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"

#include "system/lcd_touch.h"

#define TAG "POWER_CONTROL"

static void adc_bsp_init(void);

static void check_power_button_task(void *arg) {
    while (1) {
        if (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
            while (gpio_get_level(PWR_BUTTON_GPIO) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            ESP_LOGI(TAG, "Entering (deep sleep)...");

            rtc_gpio_init(PWR_BUTTON_GPIO);
            rtc_gpio_pullup_en(PWR_BUTTON_GPIO);
            rtc_gpio_pulldown_dis(PWR_BUTTON_GPIO);

            esp_sleep_enable_ext0_wakeup(PWR_BUTTON_GPIO, 0);
            lcd_panel_on(false);
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_deep_sleep_start();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void init_gpio_and_power_task() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    adc_bsp_init();

    xTaskCreate(check_power_button_task, "check_power_button_task", 2048, NULL, 5, NULL);
}

#define ADC_Calibrate

#ifdef ADC_Calibrate
  static adc_cali_handle_t cali_handle;
#endif
  static adc_oneshot_unit_handle_t adc1_handle;
static void adc_bsp_init(void)
{
#ifdef ADC_Calibrate
  adc_cali_curve_fitting_config_t cali_config = 
  {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12, //4096
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
#endif
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1, //ADC1
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
  adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_12,//ADC_ATTEN_DB_12,         //    1.1          ADC_ATTEN_DB_12:3.3
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
}
void adc_get_value(float *value,int *data)
{
  int adcdata;
#ifdef ADC_Calibrate
  int vol = 0;
#endif
  esp_err_t err;
  err = adc_oneshot_read(adc1_handle,ADC_CHANNEL_3,&adcdata);
  if(err == ESP_OK)
  {
#ifdef ADC_Calibrate
    adc_cali_raw_to_voltage(cali_handle,adcdata,&vol);
    *value = 0.001 * vol * 3;
#else
    *value = ((float)adcdata * 3.3/4096) * 3;
#endif
    *data = adcdata;
  }
  else
  {
    *value = 0;
    *data = 0;
  }
}