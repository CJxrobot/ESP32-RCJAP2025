// Example (Practice): Dual Core - LED + Serial + Serial0
// Students: Fill in the TODO parts and test on your ESP32

#define LED_BUILTIN 2   // Built-in LED pin

// Step 1: Declare the task functions
void /* TODO: TaskBlink */(void *pvParameters);
void /* TODO: TaskComm */(void *pvParameters);

void setup() {
  // Step 2: Set LED pin as output
  pinMode(LED_BUILTIN, /* TODO: OUTPUT */);

  // Step 3: Begin Serial (debug) and Serial0 (UART to another board)
  Serial.begin(/* TODO: baud rate */);
  Serial0.begin(/* TODO: baud rate */);

  // Step 4: Create a task for LED blinking on Core 0
  xTaskCreatePinnedToCore(
      /* TODO: TaskBlink */,   // Function
      "Blink",                 // Task name
      2000,                    // Stack size
      NULL,                    // Input parameter
      1,                       // Priority
      NULL,                    // Task handle
      /* TODO: core number */);

  // Step 5: Create a task for communication on Core 1
  xTaskCreatePinnedToCore(
      /* TODO: TaskComm */,
      "Comm",
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

// Task 2: Print debug + send data via Serial0
void /* TODO: TaskComm */(void *pvParameters) {
  while (1) {
    // Debug print
    Serial.println(/* TODO: message string */);

    // Send structured data over Serial0
    Serial0.write(0xAA); // Header
    uint8_t data = /* TODO: some value, e.g. LED state */;
    Serial0.write(data);
    Serial0.write(0xEE); // Footer

    vTaskDelay(/* TODO: delay time (ms) */ / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Empty, tasks are running on cores
}
