// Practice Example: IR Sensor Reading on 3 Pins
// Students: Fill in the TODO parts and test on your ESP32
// Goal: Read 3 IR sensors every 833 microseconds and show the values

// Step 1: Define 3 sensor pins
#define IR1_PIN /* TODO: pin number */
#define IR2_PIN /* TODO: pin number */
#define IR3_PIN /* TODO: pin number */

// Step 2: Setup timing variables
unsigned long lastMicros = 0;
const unsigned long interval = /* TODO: 833 */; // microseconds

// Step 3: Array to store 3 sensor values
int ir_values[/* TODO: number of sensors */];

void setup() {
  // Step 4: Set pins as input with pull-up
  pinMode(IR1_PIN, /* TODO: INPUT_PULLUP */);
  pinMode(IR2_PIN, /* TODO: INPUT_PULLUP */);
  pinMode(IR3_PIN, /* TODO: INPUT_PULLUP */);

  // Step 5: Begin Serial for debugging
  Serial.begin(/* TODO: baud rate */);
}

void loop() {
  unsigned long now = micros();

  // Step 6: Check if interval has passed
  if (now - lastMicros >= interval) {
    lastMicros = now;

    // Step 7: Read all 3 sensors (active LOW → invert with ! )
    ir_values[0] = digitalRead(/* TODO: IR1_PIN */);
    ir_values[1] = digitalRead(/* TODO: IR2_PIN */);
    ir_values[2] = digitalRead(/* TODO: IR3_PIN */);

    // Step 8: Print results
    Serial.print("IR1: ");
    Serial.print(/* TODO: first value */);
    Serial.print(",IR2: ");
    Serial.print(/* TODO: second value */);
    Serial.print(",IR3: ");
    Serial.println(/* TODO: third value */);
  }
}

