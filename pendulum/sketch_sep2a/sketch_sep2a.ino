#include <Arduino.h>
#include <ESP32Servo.h>

// Pins
int sensorPin = 34;     // IR sensor output pin
int buzzerPin = 4;      // Buzzer pin
int servoPin  = 15;     // Servo control pin
int buttonPin = 5;   

// Variables
int count = 0;          // number of oscillations
int crossings = 0;      // number of sensor triggers
int lastState = HIGH;   // store last sensor state
bool servoAt180 = false; // track servo position

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

      // Buzz on every crossing
      digitalWrite(buzzerPin, HIGH);
      vTaskDelay(pdMS_TO_TICKS(200));
      digitalWrite(buzzerPin, LOW);

      // Toggle servo position
      if (servoAt180) {
        myServo.write(0);      // go back to 0°
        servoAt180 = false;
        Serial.println("Servo moved to 0°");
      } else {
        myServo.write(180);    // go to 180°
        servoAt180 = true;
        Serial.println("Servo moved to 180°");
      }

      // Every 2 crossings = 1 oscillation
      if (crossings % 2 == 0) {
        count++;
        Serial.print("Oscillations: ");
        Serial.println(count);
      }
    }

    lastState = sensorValue;
    vTaskDelay(pdMS_TO_TICKS(10)); // small delay
  }
}
// ------------------ Button Task ------------------
void ButtonTask(void *parameter) {
  bool buzzerOn = false;     // track buzzer state
  int lastButtonState = HIGH; // pull-up, so default HIGH

  for (;;) {
    int buttonState = digitalRead(buttonPin);

    // Detect falling edge (HIGH -> LOW = button press)
    if (lastButtonState == HIGH && buttonState == LOW) {
      buzzerOn = !buzzerOn;  // toggle buzzer state

      if (buzzerOn) {
        digitalWrite(buzzerPin, HIGH);
        Serial.println("Buzzer ON (toggled)");
      } else {
        digitalWrite(buzzerPin, LOW);
        Serial.println("Buzzer OFF (toggled)");
      }

      vTaskDelay(pdMS_TO_TICKS(300)); // debounce delay after press
    }

    lastButtonState = buttonState;
    vTaskDelay(pdMS_TO_TICKS(20)); // small debounce scan
  }
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Attach servo
  myServo.attach(servoPin, 500, 2400);
  myServo.write(0); // start at 0°

  // Create FreeRTOS task
  xTaskCreatePinnedToCore(SensorTask, "Sensor Task", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(ButtonTask, "Button Task", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // Empty — FreeRTOS handles tasks
}
