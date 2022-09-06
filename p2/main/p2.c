#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define GPIO_OUTPUT_IO_0    4
#define GPIO_OUTPUT_PIN_SEL  1ULL<<GPIO_OUTPUT_IO_0
#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_PIN_SEL  1ULL<<GPIO_INPUT_IO_0
#define ESP_INTR_FLAG_DEFAULT 0

bool LED_flag = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    LED_flag = !LED_flag; //toggle led bit
    gpio_set_level(GPIO_OUTPUT_IO_0, LED_flag);
}

void app_main(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of fall edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, NULL);
    //set led = low
    gpio_set_level(GPIO_OUTPUT_IO_0, LED_flag);

    while(1) { //Aqui você pode tirar o while(1) - o micro entra em Idle.
        
    }
}
