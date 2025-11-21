#include "driver/gpio.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define ledPin GPIO_NUM_2

void app_main(void)
{
	gpio_reset_pin(ledPin);
	gpio_set_direction(ledPin, GPIO_MODE_OUTPUT);
    while (true) {
        printf("ON\n");
        gpio_set_level(ledPin, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        printf("OFF\n");
        gpio_set_level(ledPin, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
