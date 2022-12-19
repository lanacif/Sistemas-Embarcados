#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/timer.h"
#include "driver/gpio.h"

#define TIMER_DIVIDER         (16)  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

#define GPIO_OUTPUT_IO_0    4
#define GPIO_OUTPUT_PIN_SEL  1ULL<<GPIO_OUTPUT_IO_0
#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_PIN_SEL  1ULL<<GPIO_INPUT_IO_0
#define ESP_INTR_FLAG_DEFAULT 0

bool LED_flag = 0;
int Timer_selected = 1;

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
    timer_count_dir_t direction;
} example_timer_info_t;

static bool IRAM_ATTR  timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    example_timer_info_t *info = (example_timer_info_t *) args;

    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);

    if (!info->auto_reload) {
        if(info->direction == TIMER_COUNT_UP)
            timer_counter_value += info->alarm_interval * TIMER_SCALE;
        else //Contagem -= OBS:verificar se a contagem volta para o máximo ou para o setpoint
            timer_counter_value -= info->alarm_interval * TIMER_SCALE;   
        
        timer_group_set_alarm_value_in_isr(info->timer_group, info->timer_idx, timer_counter_value);
    }

    LED_flag = !LED_flag; //toggle led bit
    gpio_set_level(GPIO_OUTPUT_IO_0, LED_flag);

    return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

/**
 * @brief Configure selected timer of timer group
 *
 * @param group Timer Group number, index from 0
 * @param timer timer ID, index from 0
 * @param direction Count direction
 * @param auto_reload whether auto-reload on alarm event
 * @param timer_interval_sec interval of alarm
 * @param start_value Timer start value
 */
static void example_tg_timer_config(int group, int timer, timer_count_dir_t direction, bool auto_reload, int timer_interval_sec, uint64_t start_value)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = direction,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; // default clock source is APB
    timer_init(group, timer, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(group, timer, (uint64_t)(start_value * TIMER_SCALE));

    /* Configure the alarm value and the interrupt on alarm. */
    if (direction ==  TIMER_COUNT_UP)
        timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE);
    else
        timer_set_alarm_value(group, timer, (start_value - timer_interval_sec) * TIMER_SCALE);
    timer_enable_intr(group, timer);

    example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
    timer_info->timer_group = group;
    timer_info->timer_idx = timer;
    timer_info->auto_reload = auto_reload;
    timer_info->alarm_interval = timer_interval_sec;
    timer_info->direction = direction;
    timer_isr_callback_add(group, timer, timer_group_isr_callback, timer_info, 0);

}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    Timer_selected++;
    if(Timer_selected == 4){
        Timer_selected = 1;
    }
    if(Timer_selected == 1){
        timer_start(TIMER_GROUP_0, TIMER_0);
        timer_pause(TIMER_GROUP_1, TIMER_0);
        timer_pause(TIMER_GROUP_1, TIMER_1);
    }

    if(Timer_selected == 2){
        timer_pause(TIMER_GROUP_0, TIMER_0);
        timer_start(TIMER_GROUP_1, TIMER_0);
        timer_pause(TIMER_GROUP_1, TIMER_1);
    }

    if(Timer_selected == 3){
        timer_pause(TIMER_GROUP_0, TIMER_0);
        timer_pause(TIMER_GROUP_1, TIMER_0);
        timer_start(TIMER_GROUP_1, TIMER_1);
    }
}

void app_main(void)
{
    //Iniciando os 3 timers
    example_tg_timer_config(TIMER_GROUP_0, TIMER_0, TIMER_COUNT_UP, true, 3,0);
    example_tg_timer_config(TIMER_GROUP_1, TIMER_0, TIMER_COUNT_UP, false, 5, 0);
    example_tg_timer_config(TIMER_GROUP_1, TIMER_1, TIMER_COUNT_DOWN, false, 7, 300);

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

    //interrupt of rise edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
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

    timer_start(TIMER_GROUP_0, TIMER_0);
}