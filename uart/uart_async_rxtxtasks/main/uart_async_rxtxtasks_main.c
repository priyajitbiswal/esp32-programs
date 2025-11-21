#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT2 UART_NUM_2
#define RX2_PIN 26
#define TX2_PIN 25

static QueueHandle_t q, q1;

void taskA(void * para){
    int temp = 1;
    while(1){
        if(temp > 15){
            temp = 1;
        }
        xQueueSend(q, &temp, pdMS_TO_TICKS(10));
        temp++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void taskB(void *para){
    int item;
    while(1){
        if(xQueueReceive(q, &item, pdMS_TO_TICKS(100)) == pdTRUE){
            uint8_t num = (uint8_t)item;
            uart_write_bytes(UART_PORT2, &num, 1);
            ESP_LOGI("tag", "Sent: %d", item);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void taskC(void *para){
    int8_t rec_num;
    while(1){
        int len = uart_read_bytes(UART_PORT2, &rec_num, 1, pdMS_TO_TICKS(100));
        if(len > 0){
            ESP_LOGI("UART", "Received: %d", rec_num);
            xQueueSend(q1, &rec_num, pdMS_TO_TICKS(10));
        }
    }
}

void app_main() {
    const uart_config_t uart_config = {
        .baud_rate = 300,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_PORT2, &uart_config);
    uart_set_pin(UART_PORT2, TX2_PIN, RX2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT2, 1024, 1024, 0, NULL, 0);

    q = xQueueCreate(15, sizeof(int));
    q1 = xQueueCreate(20, sizeof(uint8_t));

    xTaskCreatePinnedToCore(taskA, "taskA", 4096, NULL, 1 , NULL, 0);
    xTaskCreatePinnedToCore(taskB, "taskB", 4096, NULL, 1 , NULL, 0);
    xTaskCreatePinnedToCore(taskC, "taskC", 4096, NULL, 1 , NULL, 0);
}
