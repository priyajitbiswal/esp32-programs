#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t xMutex;
int sharedCounter = 0;

void Task1(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      
      Serial.println("Task1 entered critical section");
      int temp = sharedCounter;
      temp++;
      vTaskDelay(200 / portTICK_PERIOD_MS); // simulate work
      sharedCounter = temp;
      Serial.print("Task1 updated value: ");
      Serial.println(sharedCounter);

      xSemaphoreGive(xMutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void Task2(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      
      Serial.println("Task2 entered critical section");
      int temp = sharedCounter;
      temp++;
      vTaskDelay(150 / portTICK_PERIOD_MS);
      sharedCounter = temp;
      Serial.print("Task2 updated value: ");
      Serial.println(sharedCounter);

      xSemaphoreGive(xMutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  xMutex = xSemaphoreCreateMutex();

  xTaskCreate(Task1, "Task1", 2048, NULL, 1, NULL);
  xTaskCreate(Task2, "Task2", 2048, NULL, 1, NULL);
}

void loop() {}
