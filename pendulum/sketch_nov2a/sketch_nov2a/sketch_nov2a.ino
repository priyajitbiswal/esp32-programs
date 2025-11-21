#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
Servo servo;
const int PIN_SERVO = 13;

const int PIN_IRSENSOR = 18;
const int PIN_BUZZER = 2;
const int PIN_PUSHBTN = 5;

const int COUNT_THRHLD = 25;
const int BUZZER_BEEP_DELAY = 50;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =================== UTILITY FUNCTIONS ===================

void beep_buzzer(int delayMs = BUZZER_BEEP_DELAY) {
  digitalWrite(PIN_BUZZER, HIGH);
  vTaskDelay(pdMS_TO_TICKS(delayMs));
  digitalWrite(PIN_BUZZER, LOW);
}

void lcd_clear() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void lcd_print(int col, int row, const char *buf) {
  lcd_clear();
  lcd.setCursor(col, row);
  lcd.print(buf);
}

void lcd_print_count(int c) {
  char buffer[17];
  snprintf(buffer, sizeof(buffer), "Count: %d", c);
  lcd_print(0, 1, buffer);
}

// =================== GLOBAL VARIABLES ===================

volatile int count = 0;
volatile bool has_pushbtn_pressed = false;
volatile bool has_detected_object = false;
volatile unsigned long last_time = 0;
volatile unsigned long period = 0;

SemaphoreHandle_t sh_buzzer;
SemaphoreHandle_t sh_pushbtn_intpt;

TaskHandle_t th_ir_sensor_read = NULL;
TaskHandle_t th_pushbtn_intpt = NULL;
TaskHandle_t th_lcd_print_count = NULL;

// =================== INTERRUPT HANDLER ===================

void IRAM_ATTR itr_handle_pushbtn() {
  if (!has_pushbtn_pressed)
    has_pushbtn_pressed = true;
}

// =================== TASKS ===================

// IR Sensor reading task
void task_ir_sensor_read(void *p) {
  bool prev_state = false;

  while (1) {
    int ir_value = digitalRead(PIN_IRSENSOR);
    bool current_state = (ir_value == LOW);  // Active low IR
  
    if (!current_state && prev_state) {
      ++count;
      has_detected_object = !has_detected_object;
      if (count >= COUNT_THRHLD) count = 0;

      unsigned long now = millis();
      if (last_time != 0)
        period = 2 * (now - last_time);
      last_time = now;

      // Create subtasks for each event
      xTaskCreatePinnedToCore(task_buzzer_beep, "Beep", 2048, NULL, 3, NULL, 1);
      xTaskCreatePinnedToCore(task_lcd_print_count, "LCD Print", 2048, NULL, 3, NULL, 1);
      xTaskCreatePinnedToCore(task_servo_dodge, "Servo Dodge", 2048, NULL, 3, NULL, 1);
    }

    prev_state = current_state;
    vTaskDelay(pdMS_TO_TICKS(10));  // 10 ms sampling
  }
}

// Buzzer task
void task_buzzer_beep(void *p) {
  if (xSemaphoreTake(sh_buzzer, pdMS_TO_TICKS(10)) == pdTRUE) {
    Serial.print("Count: ");
    Serial.println(count);
    beep_buzzer();
    xSemaphoreGive(sh_buzzer);
  }
  vTaskDelete(NULL);
}

// Pushbutton interrupt handling task
void task_pushbtn_intpt(void *p) {
  while (1) {
    if (has_pushbtn_pressed) {
      count = 0;
      lcd_print(0, 0, "Interrupt!");
      Serial.println("INTERRUPT!");
      beep_buzzer(1000);
      has_pushbtn_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// LCD update task
void task_lcd_print_count(void *p) {
  lcd_print_count(count);
  vTaskDelete(NULL);
}

// Servo "dodge" task
void task_servo_dodge(void *p) {
  // Servo dodges when pendulum detected
  if(has_detected_object) servo.write(180);
  else servo.write(90); 
  vTaskDelete(NULL);
}

// =================== SETUP ===================

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Initialize servo
  servo.attach(PIN_SERVO);
  servo.write(90);  // center position

  // Pin setup
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_IRSENSOR, INPUT);
  pinMode(PIN_PUSHBTN, INPUT_PULLUP);

  // LCD setup
  lcd.init();
  lcd.backlight();

  // Semaphores
  sh_buzzer = xSemaphoreCreateMutex();
  sh_pushbtn_intpt = xSemaphoreCreateBinary();

  // Interrupt setup
  attachInterrupt(digitalPinToInterrupt(PIN_PUSHBTN), itr_handle_pushbtn, FALLING);

  // Tasks
  xTaskCreatePinnedToCore(task_ir_sensor_read, "ir_sensor_read", 2048, NULL, 2, &th_ir_sensor_read, 1);
  xTaskCreatePinnedToCore(task_pushbtn_intpt, "pushbtn_intpt", 2048, NULL, 1, &th_pushbtn_intpt, 1);
}

void loop() {}