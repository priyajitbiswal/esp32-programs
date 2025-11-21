#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pins
int sensorPin = 34;
int buzzerPin = 4;
int servoPin  = 15;
int buttonPin = 5;

// Variables
int count = 0;
int crossings = 0;
int lastState = HIGH;
bool servoAt180 = false;

// Servo
Servo myServo;

// Pause flag
volatile bool paused = false;

// Debounce timing
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;  // 200 ms debounce

// LCD: 0x27 is the I2C address, 16x2 display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ISR for button press (toggle pause state with debounce)
void IRAM_ATTR handleButtonPress() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay) {
    paused = !paused;   // toggle pause state
    lastInterruptTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  myServo.attach(servoPin, 500, 2400);
  myServo.write(0);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Pendulum Ready");
  lcd.setCursor(0, 1);
  lcd.print("Cross:0 Osc:0");
}

void loop() {
  if (paused) {
    digitalWrite(buzzerPin, HIGH);   // buzzer ON while paused
    lcd.setCursor(0, 0);
    lcd.print("   PAUSED MODE   ");
    crossings = 0; 
    count = 0; 
    //vTaskDelay(pdMS_TO_TICKS(10));
    return;
  } else {
    digitalWrite(buzzerPin, LOW);
    lcd.setCursor(0, 0);
    lcd.print("   RUNNING...    ");
  }

  int sensorValue = digitalRead(sensorPin);

  // Detect crossing: HIGH -> LOW
  if (lastState == HIGH && sensorValue == LOW) {
    if (crossings == 25) { crossings = 0; count = 0; }
    crossings++;
    Serial.print("Crossing detected: ");
    Serial.println(crossings);

    // Buzz on every crossing
    digitalWrite(buzzerPin, HIGH);
    vTaskDelay(pdMS_TO_TICKS(200));
    digitalWrite(buzzerPin, LOW);

    // Toggle servo position
    if (servoAt180) {
      myServo.write(0);
      servoAt180 = false;
      Serial.println("Servo moved to 0°");
    } else {
      myServo.write(180);
      servoAt180 = true;
      Serial.println("Servo moved to 180°");
    }

    // Every 2 crossings = 1 oscillation
    if (crossings % 2 == 0) {
      count++;
      Serial.print("Oscillations: ");
      Serial.println(count);
    }

    // Update LCD
    lcd.setCursor(0, 1);
    lcd.print("Cross:");
    lcd.print(crossings);
    lcd.print(" Osc:");
    lcd.print(count);
    lcd.print("   "); // clear leftover chars
  }

  lastState = sensorValue;
  vTaskDelay(pdMS_TO_TICKS(10));
}
