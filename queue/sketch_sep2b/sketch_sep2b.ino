#include <Arduino.h>

// Optional: Only use one core if UNICORE is defined
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Queue
static QueueHandle_t msgqueue;
static const uint8_t msgqueue_len = 5;

// Task to receive and print messages
void printmsg(void *parameter)
{
    int item;
    while (1)
    {
        if (xQueueReceive(msgqueue, &item, portMAX_DELAY) == pdTRUE)
        {
            Serial.print("Received: ");
            Serial.println(item);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000); // Give Serial some time to start

    msgqueue = xQueueCreate(msgqueue_len, sizeof(int));
    if (msgqueue == NULL)
    {
        Serial.println("Queue creation failed!");
        while (1)
            ; // Halt
    }

    xTaskCreatePinnedToCore(
        printmsg,     // Task function
        "Print Task", // Name
        2048,         // Stack size (in words)
        NULL,         // Parameters
        1,            // Priority
        NULL,         // Task handle
        app_cpu       // Core to run on
    );
}

void loop()
{
    static int num = 0;
    if (xQueueSend(msgqueue, &num, 10) != pdTRUE)
    {
        Serial.println("Queue full! Couldn't send.");
    }
    else
    {
        Serial.print("Sent: ");
        Serial.println(num);
    }

    num++;
    vTaskDelay(400 / portTICK_PERIOD_MS); // <-- Not yTaskDelay
}