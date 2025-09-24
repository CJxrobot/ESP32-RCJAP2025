#include <Preferences.h>

// ============================
// Student Template: Average Calculation + EEPROM Storage
// ============================

// Number of sensors
#define SENSOR_COUNT /* TODO: number of sensors */

// Analog pin numbers
uint8_t pin_config[SENSOR_COUNT] = { /* TODO: analog pins */ };

// Arrays for readings
uint16_t max_val[SENSOR_COUNT];
uint16_t min_val[SENSOR_COUNT];
uint16_t avg_val[SENSOR_COUNT];

// Temporary reading array
uint16_t result[SENSOR_COUNT];

// Preferences object for EEPROM storage
Preferences memory;

// ============================
// Setup
// ============================
void setup() {
  Serial.begin(/* TODO: baud rate */);

  // Initialize sensor pins
  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    pinMode(pin_config[i], /* TODO: INPUT */);
    max_val[i] = 0;
    min_val[i] = 4095;  // assume 12-bit ADC
  }

  // Initialize EEPROM/Preferences
  memory.begin(/* TODO: namespace string */, false);

  // Optionally load previous averages
  memory.getBytes(/* TODO: key string */, avg_val, memory.getBytesLength(/* TODO: key string */));

  Serial.println("Setup complete.");
}

// ============================
// Function: Scan sensors and update max/min
// ============================
void scan_sensors() {
  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    // TODO: read analog value
    uint16_t reading = /* TODO: analogRead(pin_config[i]) */;

    if (reading > max_val[i]) max_val[i] = reading;
    if (reading < min_val[i]) min_val[i] = reading;
  }
}

// ============================
// Function: Calculate average and save to EEPROM
// ============================
void save_average() {
  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    avg_val[i] = (max_val[i] + min_val[i]) / 2;

    // Debug print
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": max=");
    Serial.print(max_val[i]);
    Serial.print(", min=");
    Serial.print(min_val[i]);
    Serial.print(", avg=");
    Serial.println(avg_val[i]);
  }

  // Save to EEPROM/Preferences
  memory.putBytes(/* TODO: key string */, avg_val, sizeof(avg_val));
  Serial.println("Averages saved to EEPROM.");
}

// ============================
// Loop
// ============================
void loop() {
  // TODO: call scan_sensors() periodically
  // TODO: call save_average() when needed
  // e.g., after several scans or a button press

  delay(/* TODO: delay ms */);
}

