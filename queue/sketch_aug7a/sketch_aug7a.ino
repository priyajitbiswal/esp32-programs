#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#if CONFIG_FREERTOS_UNICORE

QueueHandle_t msqueue;  // Global queue handle
int item = 0;           // Global item to receive into

// Task to receive from the queue and print
void printmsg(void *parameter) {
  while (1) {
    if (xQueueReceive(msqueue, (void *)&item, 0) == pdTRUE) {
      Serial.print("Received from queue: ");
      Serial.println(item);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay 1 second
  }
}

// Task to send to the queue
void sendmsg(void *parameter) {
  int count = 0;
  while (1) {
    if (xQueueSend(msqueue, (void *)&count, portMAX_DELAY) == pdTRUE) {
      Serial.print("Sent to queue: ");
      Serial.println(count);
      count++;
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay 2 seconds
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial to start

  // Create a queue of 10 integers
  msqueue = xQueueCreate(10, sizeof(int));
  if (msqueue == NULL) {
    Serial.println("Queue creation failed");
    while (1);
  }

  // Create tasks
  xTaskCreate(printmsg, "PrintTask", 2048, NULL, 1, NULL);
  xTaskCreate(sendmsg, "SendTask", 2048, NULL, 1, NULL);
}

void loop() {
  // Nothing needed here, tasks run independently
}

#endif
