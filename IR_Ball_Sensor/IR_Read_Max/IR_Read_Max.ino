// Practice Example: 5 IR Sensors - Find Maximum Weight
// Students: Fill in the TODO parts
// Goal: Use 5 sensors, each mapped to a degree, 
// find which sensor has the highest weight and print its degree.

#define IR_COUNT /* TODO: 5 */

// Step 1: Define sensor pins
int ir_pins[IR_COUNT] = { /* TODO: fill pin numbers */ };

// Step 2: Define corresponding sensor degrees
int ir_degrees[IR_COUNT] = { /* TODO: e.g. 0, 72, 144, 216, 288 */ };

// Step 3: Arrays to store weights
int ir_weight[IR_COUNT];

void setup() {
  // Step 4: Set all pins as input
  for (int i = 0; i < IR_COUNT; i++) {
    pinMode(ir_pins[i], /* TODO: INPUT_PULLUP */);
  }

  Serial.begin(/* TODO: baud rate */);
}

void loop() {
  // Reset weights
  for (int i = 0; i < IR_COUNT; i++) {
    ir_weight[i] = 0;
  }

  unsigned long start = micros();

  // Step 5: Measure for fixed time (e.g. 1000 us)
  while (micros() - start < /* TODO: time in us */) {
    for (int i = 0; i < IR_COUNT; i++) {
      int value = !digitalRead(ir_pins[i]);  // active LOW
      if (value) {
        ir_weight[i]++;  // increase weight if active
      }
    }
  }

  // Step 6: Find maximum weight
  int max_index = 0;
  for (int i = 1; i < IR_COUNT; i++) {
    if (ir_weight[i] > ir_weight[max_index]) {
      max_index = i;
    }
  }

  // Step 7: Print result
  Serial.print("Max Weight = ");
  Serial.print(ir_weight[max_index]);
  Serial.print(" at Degree = ");
  Serial.println(ir_degrees[max_index]);

  delay(/* TODO: delay time (ms) */);
}
