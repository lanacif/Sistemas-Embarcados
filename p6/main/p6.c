/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/timer.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

#define GPIO_OUTPUT_IO_0    2
#define GPIO_OUTPUT_PIN_SEL  1ULL<<GPIO_OUTPUT_IO_0

#define TIMER_DIVIDER         (16)  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle fila_tx_valor_att = NULL;

bool LED_flag = 0;

static bool IRAM_ATTR  timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;

    LED_flag = !LED_flag; //toggle led bit
    gpio_set_level(GPIO_OUTPUT_IO_0, LED_flag);

    return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    int recebido_rx;
    char *msg = malloc(RX_BUF_SIZE+51);

    while (1) {
        if(xQueueReceive(fila_tx_valor_att, &recebido_rx, portMAX_DELAY)) {
            sprintf(msg, "Timer atualizado, tempo p/ LED ligado/desligado: %ds\n", recebido_rx);
            sendData(TX_TASK_TAG, msg);
        }
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int tempo_ligado;

    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            //Pausa timer
            timer_pause(TIMER_GROUP_0, TIMER_0);
            //Seta o timer count pra 0
            timer_set_counter_value(TIMER_GROUP_0, TIMER_0, (uint64_t)0);
            //Converte o valor recebido em inteiro
            tempo_ligado = atoi((char*)data);
            //Seta o alarme para o valor recebido
            timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, tempo_ligado * TIMER_SCALE);
            //Starta o timer
            timer_start(TIMER_GROUP_0, TIMER_0);

            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            xQueueSendToBack(fila_tx_valor_att, &tempo_ligado, portMAX_DELAY);
        }
    }
    free(data);
}

void app_main(void)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

   /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, (uint64_t)0);

    //o alarme deve mudar para a frequencia recebida (Iniciando em 1 hz)
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1 * TIMER_SCALE);

    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_group_isr_callback, NULL, 0);

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE, //disable interrupt
        .mode = GPIO_MODE_OUTPUT, //set as output mode
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL, //bit mask of the pins
        .pull_down_en = 0, //disable pull-down mode
        .pull_up_en = 0, //disable pull-up mode
    };

    //configure GPIO with the given settings
    gpio_config(&io_conf);

    init();
    //Fila para retornar o valor att
    fila_tx_valor_att = xQueueCreate(5, sizeof(int));
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);

    //Starta o timer
    timer_start(TIMER_GROUP_0, TIMER_0);
}