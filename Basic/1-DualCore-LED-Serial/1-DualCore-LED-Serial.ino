// Example 1: Dual Core - LED + Serial Print

#define LED_BUILTIN 2

void TaskBlink(void *pvParameters);
void TaskPrint(void *pvParameters);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // Task on Core 0: LED blinking
  xTaskCreatePinnedToCore(TaskBlink, "Blink", 2000, NULL, 1, NULL, 0);

  // Task on Core 1: Serial printing
  xTaskCreatePinnedToCore(TaskPrint, "Print", 2000, NULL, 1, NULL, 1);
}

void TaskBlink(void *pvParameters) {
  while (1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void TaskPrint(void *pvParameters) {
  while (1) {
    Serial.println("Hello from Core 1");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Empty
}

