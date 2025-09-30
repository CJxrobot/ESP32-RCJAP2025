// Example 1 (Practice): Dual Core - LED + Serial Print
// Students: Fill in the TODO parts and test on your ESP32

#define LED_BUILTIN 2   // Built-in LED pin

// Step 1: Declare the task functions
void /* TODO: TaskBlink */(void *pvParameters);
void /* TODO: TaskPrint */(void *pvParameters);

void setup() {
  // Step 2: Set LED pin as output
  pinMode(LED_BUILTIN, /* TODO: OUTPUT */);

  // Step 3: Begin Serial communication at 115200 baud
  Serial.begin(/* TODO: baud rate */);

  // Step 4: Create a task for LED blinking on Core 0
  xTaskCreatePinnedToCore(
      /* TODO: TaskBlink */,   // Function
      "Blink",                 // Task name
      2000,                    // Stack size
      NULL,                    // Input parameter
      1,                       // Priority
      NULL,                    // Task handle
      /* TODO: core number */);

  // Step 5: Create a task for printing on Core 1
  xTaskCreatePinnedToCore(
      /* TODO: TaskPrint */,
      "Print",
      2000,
      NULL,
      1,
      NULL,
      /* TODO: core number */);
}

// Task 1: Blink LED
void /* TODO: TaskBlink */(void *pvParameters) {
  while (1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle LED
    vTaskDelay(/* TODO: delay time (ms) */ / portTICK_PERIOD_MS);
  }
}

// Task 2: Print message
void /* TODO: TaskPrint */(void *pvParameters) {
  while (1) {
    Serial.println(/* TODO: message string */);
    vTaskDelay(/* TODO: delay time (ms) */ / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Empty, tasks are running on cores
}
