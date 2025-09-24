// ============================
// Student Template: Dual Core + 5 Analog Readings
// Fill in all TODO parts
// ============================

// Define number of sensors
#define SENSOR_COUNT /* TODO: number of sensors */

uint8_t pin_config[SENSOR_COUNT] = { /* TODO: analog pins */ };
uint16_t result[SENSOR_COUNT]; 

TaskHandle_t Task1;
TaskHandle_t Task2;

bool read_done_flag = false;  // Shared flag between tasks

// ============================
// Setup
// ============================
void setup() {
  Serial.begin(/* TODO: baud rate */);    // Debug
  Serial0.begin(/* TODO: baud rate */);   // Data sending
  Serial.println("init_done");

  // Create Task1 on Core 0: sensor reading
  xTaskCreatePinnedToCore(
    /* TODO: Task1 function name */,
    "Task1",
    /* TODO: stack size */,
    NULL,
    /* TODO: priority */,
    &Task1,
    0);

  // Create Task2 on Core 1: printing/sending
  xTaskCreatePinnedToCore(
    /* TODO: Task2 function name */,
    "Task2",
    /* TODO: stack size */,
    NULL,
    /* TODO: priority */,
    &Task2,
    1);
}

// ============================
// Task1: Read analog sensors
// ============================
void /* TODO: Task1 function name */(void * parameter) {
  while (true) {
    read_done_flag = false;

    // TODO: loop through pins and perform analogRead
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
      result[i] = /* TODO: analogRead(pin_config[i]) */;
    }

    read_done_flag = true;
    vTaskDelay(/* TODO: delay in ms */ / portTICK_PERIOD_MS);
  }
}

// ============================
// Task2: Print + Send data
// ============================
void /* TODO: Task2 function name */(void * parameter) {
  while (true) {
    if (read_done_flag) {
      // Debug print
      Serial.print("Readings: ");
      for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        Serial.print(/* TODO: result[i] */);
        Serial.print(" ");
      }
      Serial.println();

      // Send via Serial0
      Serial0.print(/* TODO: start marker */);
      for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        Serial0.print(/* TODO: result[i] */);
        Serial0.print(/* TODO: separator */);
      }
      Serial0.println(/* TODO: end marker */);
    }
    vTaskDelay(/* TODO: delay in ms */ / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Empty - tasks handle everything
}

