#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

//Timer do PWM, High speed, gpio PWM, Canal
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (12)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

//ADC Channels
#define ADC1_EXAMPLE_CHAN0          ADC1_CHANNEL_6

//static const char *TAG_CH = {"ADC1_CH6"};

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11

static esp_adc_cal_characteristics_t adc1_chars;

void app_main(void)
{
    //Config timer pwm
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT, // resolution of PWM duty
        .freq_hz = 10000,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set config do timer
    ledc_timer_config(&ledc_timer);

    //Config do canal PWM
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER,
        .flags.output_invert = 0
    };
    //Set do canal do pwm
    ledc_channel_config(&ledc_channel);

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

    while (1) {
        //ESP_LOGI(TAG_CH, "raw  data: %d", adc1_get_raw(ADC1_EXAMPLE_CHAN0));
        //ESP_LOGI(TAG_CH, "cali data: %d mV", esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_EXAMPLE_CHAN0), &adc1_chars));
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, adc1_get_raw(ADC1_EXAMPLE_CHAN0));
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}