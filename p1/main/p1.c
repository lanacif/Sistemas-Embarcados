#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void app_main(void)
{
    esp_reset_reason_t reset_reason =  esp_reset_reason();
    switch(reset_reason){
    case ESP_RST_UNKNOWN:
        printf("Reset reason can not be determined.\n");
        break;
    case ESP_RST_POWERON:
        printf("Reset due to power-on event.\n");
        break;
    case ESP_RST_EXT:
        printf("Reset by external pin (not applicable for ESP32)\n");
        break;
    case ESP_RST_SW:
        printf("Software reset via esp_restart.\n");
        break;
    case ESP_RST_PANIC:
        printf("Software reset due to exception/panic.\n");
        break;
    case ESP_RST_INT_WDT:
        printf("Reset (software or hardware) due to interrupt watchdog.\n");
        break;
    case ESP_RST_TASK_WDT:
        printf("Reset due to task watchdog.\n");
        break;
    case ESP_RST_WDT:
        printf("Reset due to other watchdogs.\n");
        break;
    case ESP_RST_DEEPSLEEP:
        printf("Reset after exiting deep sleep mode.\n");
        break;
    case ESP_RST_BROWNOUT:
        printf("Brownout reset (software or hardware).\n");
        break;
    case ESP_RST_SDIO:
        printf("Reset over SDIO.\n");
        break;
    }

    printf("IDF Version: %s\n",esp_get_idf_version());

    for (int i = 3; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
