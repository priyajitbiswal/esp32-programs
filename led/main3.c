#include "soc/gpio_periph.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "driver/periph_ctrl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define led 2
#define led2 4

//task 1
/*void app_main(void)
{
	//PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[led], PIN_FUNC_GPIO);
	GPIO.enable_w1ts = (1 << led);
	
	while(1){
		GPIO.out_w1ts = (1 << led);
		vTaskDelay(500/portTICK_PERIOD_MS);
		
		GPIO.out_w1tc = (1 << led);
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}*/

//task2
void app_main(void){
	GPIO.enable_w1ts = (1 << led);
	GPIO.enable_w1ts = (1 << led2);
	
	while(1){
		GPIO.out_w1ts = (1 << led);
		GPIO.out_w1tc = (1 << led2);
		vTaskDelay(500/portTICK_PERIOD_MS);
		
		GPIO.out_w1ts = (1 << led2);
		GPIO.out_w1tc = (1 << led);
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}
