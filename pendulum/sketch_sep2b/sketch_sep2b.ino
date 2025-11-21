#include <Arduino.h>
#include <ESP32Servo.h>

// Pins
int sensorPin = 34;     // IR sensor output pin
int buzzerPin = 4;      // Buzzer pin
int servoPin  = 15;     // Servo control pin

// Variables
int count = 0;          // number of oscillations
int crossings = 0;      // number of sensor triggers
int lastState = HIGH;   // store last sensor state

// Servo object
Servo myServo;

// ------------------ Sensor Task ------------------
void SensorTask(void *parameter) {
  for (;;) {
    int sensorValue = digitalRead(sensorPin);

    // Detect change: HIGH -> LOW (pendulum crossing)
    if (lastState == HIGH && sensorValue == LOW) {
      if (crossings == 25) {
        crossings = 0;
        count = 0;
      }

      crossings++;
      Serial.print("Crossing detected: ");
      Serial.println(crossings);

      // Buzz + rotate servo
      digitalWrite(buzzerPin, HIGH);
      myServo.write(0);      // Move to 0 degrees
      vTaskDelay(200 / portTICK_PERIOD_MS);

      digitalWrite(buzzerPin, LOW);
      myServo.write(90);     // Move to 90 degrees
      vTaskDelay(200 / portTICK_PERIOD_MS);

      // Every 2 crossings = 1 oscillation
      if (crossings % 2 == 0) {
        count++;
        Serial.print("Oscillations: ");
        Serial.println(count);
      }
    }

    lastState = sensorValue;
    vTaskDelay(10 / portTICK_PERIOD_MS); // small delay
  }
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Attach servo
  myServo.attach(servoPin, 500, 2400); // 500–2400 µs pulse range
  myServo.write(90); // start centered

  // Create FreeRTOS task
  xTaskCreatePinnedToCore(SensorTask, "Sensor Task", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // Empty — FreeRTOS handles tasks
}
