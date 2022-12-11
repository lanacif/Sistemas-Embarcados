#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#include "esp_log.h"


//Timer do PWM, High speed, gpio PWM, Canal
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (12)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

//canal do ADC
#define ADC_POT                ADC1_CHANNEL_6

//Atenuação do ADC
#define ADC_POT_ATEN           ADC_ATTEN_DB_11

static const char *TAG = "MQTT";
static const char *TAG2 = "Trabalho Final";

static xQueueHandle fila_pwm = NULL;
static xQueueHandle fila_adc = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    int valor_convertido;
    char *string_sem_lixo = malloc(4);

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "velocidade", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        snprintf(string_sem_lixo, 4, "%.*s", event->data_len, event->data);
        valor_convertido = atoi(string_sem_lixo);
        xQueueSendFromISR(fila_adc, &valor_convertido, NULL);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri =  "mqtt://device_1:device_1@node02.myqtthub.com:1883",
        .client_id = "device_1",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

static void adc_task(void* arg)
{
    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_POT, ADC_POT_ATEN));
    
    int valor_recebido_mqtt;
    float limite_adc;
    uint32_t valor_ajustado;
    int msg_id;

    for(;;) {
        if(xQueueReceive(fila_adc, &valor_recebido_mqtt, portMAX_DELAY)) {
            //Aplica a correção no valor lido do adc (ax+b)
            limite_adc = 0.0415*adc1_get_raw(ADC_POT) + 200;

            //Casas decimais desconsideradas float -> uint32
            valor_ajustado = (limite_adc-200)/(370-200)*(valor_recebido_mqtt-200) + 200;

            //Publica
            //msg_id = esp_mqtt_client_publish(client, "adc", "data_3", 0, 1, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            //xQueueSend() é equivalente a xQueueSendToBack()
            xQueueSendToBack(fila_pwm, &valor_ajustado, portMAX_DELAY);
        }
    }
}

static void pwm_task(void* arg)
{
    //Configurações do Timer que será usado para o PWM
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_12_BIT,    //Resolução do duty
        .freq_hz = 50,                        //Frequência do PWM
        .speed_mode = LEDC_HS_MODE,              //Modo do timer
        .timer_num = LEDC_HS_TIMER,              //índice do timer
        .clk_cfg = LEDC_AUTO_CLK,                //Seleção automática da fonte de clock
    };
    // Set config do timer
    ledc_timer_config(&ledc_timer);

    //Configurações do canal que será usado para o PWM
    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 200,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER,
        .flags.output_invert = 1
    };
    
    //Set do canal do PWM
    ledc_channel_config(&ledc_channel);
    
    uint32_t valor_convertido_adc;

    for(;;) {
        if(xQueueReceive(fila_pwm, &valor_convertido_adc, portMAX_DELAY)) {
            //Escreve o novo pwm
            printf("Valor recebido na task PWM: %d\n",valor_convertido_adc);
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, valor_convertido_adc);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        }
    }
}

void app_main(void)
{
    //Criando a filas
    fila_pwm = xQueueCreate(5, sizeof(uint32_t));
    fila_adc = xQueueCreate(5, sizeof(int));
    //Starta a task do pwm
    xTaskCreate(pwm_task, "pwm_task", 2048, NULL, 10, NULL);
    //Starta a task do adc
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 10, NULL);

    //Print de infos
    ESP_LOGI(TAG2, "[APP] Startup..");
    ESP_LOGI(TAG2, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG2, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //Start NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    //Start netif
    ESP_ERROR_CHECK(esp_netif_init());
    //Start do event loop para ser usado pelo MQTT / WIFI
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Conexão wifi usando o example_connect para facilitar
    ESP_ERROR_CHECK(example_connect());

    //Chama a função que inicia o MQTT
    mqtt_app_start();

    //A main "morre"
}