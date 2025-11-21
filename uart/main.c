#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdlib.h>

#define threshold 35

#define UART_PORT UART_NUM_1
#define TX_PIN 17
#define RX_PIN 16

#define UART_PORT2 UART_NUM_2
#define RX2_PIN 26
#define TX2_PIN 25

#define LED 2

static QueueHandle_t q, q1;

TaskHandle_t taskD = NULL;

void taska(void * para){
	//int temp = 0;
	int min = 25;
	int max = 40;
	while(1){
		//temp++;
		//if(temp > 15)temp = 1;
		
		int temp = (rand() % (max - min + 1)) + min;
		xQueueSend(q, (void *)&temp, 10);
		
		vTaskDelay(2000/portTICK_PERIOD_MS);
	}
}

void taskb(void *para){
	int item;
	while(1){
		if (xQueueReceive(q, (void *)&item, 0)==pdTRUE){
			uint8_t num = (uint8_t)item; 
			uart_write_bytes(UART_PORT2, &num, 1);
			ESP_LOGI("tag", "Sent: %d\n", item);
		}
	vTaskDelay(2000/portTICK_PERIOD_MS);
	}
}

void taskc(void *para){
	while(1){
		uint8_t rec_num;
		int len = uart_read_bytes(UART_PORT, &rec_num, 1, 100 / portTICK_PERIOD_MS);
	    if (len > 0) {
	        ESP_LOGI("UART", "Received: %d", rec_num);
	        xQueueSend(q1, (void *)&rec_num, 10);
	    }
    }		
}	

void taskd(void *para){
	uint8_t samples[10] = {0};
	uint8_t num;
	int i = 0, c = 0;
	
	while(1){
		if (xQueueReceive(q1, &num, portMAX_DELAY)==pdTRUE){
			samples[i] = num;
			if (c < 10) c++;
			
			int sum = 0;
			for (int j = 0; j < c; j++) sum += samples[j];
				
			
			float avg = (float) sum / c;
			ESP_LOGI("avg", "Average: %f", avg);
			
			if(num > threshold) gpio_set_level(LED, 1);
			else gpio_set_level(LED, 0);
			i = (i % 10) + 1;
		}
	}
}


void app_main() {
    const uart_config_t uart_config = {
        .baud_rate = 4800,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, 1024, 1024, 0, NULL, 0);

	uart_param_config(UART_PORT2, &uart_config);
    uart_set_pin(UART_PORT2, TX2_PIN, RX2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT2, 1024, 1024, 0, NULL, 0);

	gpio_reset_pin(LED);
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);

	q = xQueueCreate(10, sizeof(int));
	q1 = xQueueCreate(20, sizeof(int));
	
	xTaskCreatePinnedToCore(taskc, "taskc", 4096, NULL, 1 , NULL, 0);
	xTaskCreatePinnedToCore(taska, "taska", 4096, NULL, 1 , NULL, 0);
	xTaskCreatePinnedToCore(taskb, "taskb", 4096, NULL, 1 , NULL, 0);
	xTaskCreatePinnedToCore(taskd, "taskd", 4096, NULL, 1 , &taskD, 0);
}
