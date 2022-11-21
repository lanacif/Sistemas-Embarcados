#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

//Botão 0
#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_PIN_SEL  1ULL<<GPIO_INPUT_IO_0
#define ESP_INTR_FLAG_DEFAULT 0

//Novo timer / canal / gpio
#define NOVO_TIMER_HS          LEDC_TIMER_1
#define NOVO_HS_CH4_CHANNEL    LEDC_CHANNEL_4
#define NOVO_HS_CH4_GPIO       (23)

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (19)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE

#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (5)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM       (4)                  //Quantidade de canais
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (3000)

static xQueueHandle gpio_evt_queue = NULL;

/*
 * This callback function will be called when fade operation has ended
 * Use callback only if you are aware it is being called inside an ISR
 * Otherwise, you can use a semaphore to unblock tasks
 */
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT) {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t) user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    //uint32_t duty_atual = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, arg, NULL);
}

static void gpio_task_example(void* arg)
{
    ledc_channel_config_t *canal = calloc(1, sizeof(ledc_channel_config_t));
    int teste = 0;
    
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, canal, portMAX_DELAY)) {
            //Se duty_atual < 1000 duty = duty + 100
            if(teste < 1000){
                teste += 100;
                ledc_set_duty(canal->speed_mode, canal->channel, teste);
                ledc_update_duty(canal->speed_mode, canal->channel);
            }
            else //Se não set duty = 0
            {
                teste = 0;
                ledc_set_duty(canal->speed_mode, canal->channel, teste);
                ledc_update_duty(canal->speed_mode, canal->channel);
            }
            printf("Duty no gpio 2: %d \n", teste);
        }
    }
}

void app_main(void)
{
    int ch;

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    //Novo timer config
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 10000,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = NOVO_TIMER_HS,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set conf do novo timer
    ledc_timer_config(&ledc_timer);

    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    ledc_timer.freq_hz = 5000,                      // frequency of PWM signal
    ledc_timer.speed_mode = LEDC_LS_MODE,           // timer mode
    ledc_timer.timer_num = LEDC_LS_TIMER,            // timer index
    ledc_timer.clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);

    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .channel    = LEDC_HS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER,
            .flags.output_invert = 0
        },
        {
            .channel    = LEDC_HS_CH1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER,
            .flags.output_invert = 0
        },
        {
            .channel    = LEDC_LS_CH2_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH2_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER,
            .flags.output_invert = 1
        },
        {
            .channel    = LEDC_LS_CH3_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH3_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER,
            .flags.output_invert = 1
        },
    };

    //Novo canal
    ledc_channel_config_t novo_canal = {
        .channel    = NOVO_HS_CH4_CHANNEL,
        .duty       = 0,
        .gpio_num   = NOVO_HS_CH4_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = NOVO_TIMER_HS,
        .flags.output_invert = 0
    };

    //Set do novo canal
    ledc_channel_config(&novo_canal);

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);
    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event
    };

    SemaphoreHandle_t counting_sem = xSemaphoreCreateCounting(LEDC_TEST_CH_NUM, 0);

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_cb_register(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, &callbacks, (void *) counting_sem);
    }

    //Struct para configurar gpio do botão
    gpio_config_t io_conf = {
        .intr_type        = GPIO_INTR_NEGEDGE,
        .mode             = GPIO_MODE_INPUT,
        .pin_bit_mask     = GPIO_INPUT_PIN_SEL,
        .pull_down_en     = 0,
        .pull_up_en       = 1
    };

    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(5, sizeof(ledc_channel_config_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, &novo_canal);

    while (1) {
        printf("1. LEDC fade up to duty = %d\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++) {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }

        printf("2. LEDC fade down to duty = 0\n");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, 0, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++) {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }

        printf("3. LEDC set duty = %d without fade\n", LEDC_TEST_DUTY);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, LEDC_TEST_DUTY);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("4. LEDC set duty = 0 without fade\n");
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, 0);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
