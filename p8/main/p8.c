#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_system.h" 
#include "driver/uart.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "spi_eeprom.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#  ifdef CONFIG_EXAMPLE_USE_SPI1_PINS
#    define EEPROM_HOST    SPI1_HOST
// Use default pins, same as the flash chip.
#    define PIN_NUM_MISO 7
#    define PIN_NUM_MOSI 8
#    define PIN_NUM_CLK  6
#  else
#    define EEPROM_HOST    HSPI_HOST
#    define PIN_NUM_MISO 18
#    define PIN_NUM_MOSI 23
#    define PIN_NUM_CLK  19
#  endif

#  define PIN_NUM_CS   13
#elif defined CONFIG_IDF_TARGET_ESP32S2
#  define EEPROM_HOST    SPI2_HOST

#  define PIN_NUM_MISO 37
#  define PIN_NUM_MOSI 35
#  define PIN_NUM_CLK  36
#  define PIN_NUM_CS   34
#elif defined CONFIG_IDF_TARGET_ESP32C3
#  define EEPROM_HOST    SPI2_HOST

#  define PIN_NUM_MISO 2
#  define PIN_NUM_MOSI 7
#  define PIN_NUM_CLK  6
#  define PIN_NUM_CS   10

#elif CONFIG_IDF_TARGET_ESP32S3
#  define EEPROM_HOST    SPI2_HOST

#  define PIN_NUM_MISO 13
#  define PIN_NUM_MOSI 11
#  define PIN_NUM_CLK  12
#  define PIN_NUM_CS   10
#endif

static const char TAG[] = "main";
static const char *RX_TASK_TAG = "RX_UART";

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)

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
    uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

//Função para saber o tamanho do dado recebido no usb p/ controle do for de escrita/leitura da eeprom
int size(uint8_t *str){
     int char_count = 0;
     while (*str != '\0'){
          char_count++;
          str++;
     }
     return char_count;
}

void app_main(void)
{
    init(); //Inicia UART
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

    esp_err_t ret;
#ifndef CONFIG_EXAMPLE_USE_SPI1_PINS
    ESP_LOGI(TAG, "Initializing bus SPI%d...", EEPROM_HOST+1);
    spi_bus_config_t buscfg={
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(EEPROM_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
#else
    ESP_LOGI(TAG, "Attach to main flash bus...");
#endif

    eeprom_config_t eeprom_config = {
        .cs_io = PIN_NUM_CS,
        .host = EEPROM_HOST,
        .miso_io = PIN_NUM_MISO,
    };
#ifdef CONFIG_EXAMPLE_INTR_USED
    eeprom_config.intr_used = true;
    gpio_install_isr_service(0);
#endif

    eeprom_handle_t eeprom_handle;

    ESP_LOGI(TAG, "Initializing device...");
    ret = spi_eeprom_init(&eeprom_config, &eeprom_handle);
    ESP_ERROR_CHECK(ret);

    ret = spi_eeprom_write_enable(eeprom_handle);
    ESP_ERROR_CHECK(ret);

    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        
            ESP_LOGI(TAG, "Escrito no AT93C46D: %s", data);
            for (int i = 0; i < size(data); i++) {
                // No need for this EEPROM to erase before write.
                ret = spi_eeprom_write(eeprom_handle, i, data[i]);
                ESP_ERROR_CHECK(ret);
            }

            uint8_t test_buf[1025] = "";
            for (int i = 0; i < size(data); i++) {
                ret = spi_eeprom_read(eeprom_handle, i, &test_buf[i]);
                ESP_ERROR_CHECK(ret);
            }
            ESP_LOGI(TAG, "Lido do AT93C46D: %s", test_buf);
        
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    free(data);
}
