// Practice Template: Dual Core IR Sensor Max Finder + Serial0
// Students: Fill in the TODO parts

#define IR_Pin_Count /* TODO: number of sensors */

uint8_t ir_pins[IR_Pin_Count]   = { /* TODO: pin numbers */ };
uint16_t ir_degree[IR_Pin_Count] = { /* TODO: degrees */ };

uint16_t ir_weight[IR_Pin_Count];
volatile bool read_done_flag = false;

// ============ IR Reading ============
bool IR_reading() {
  for (int i = 0; i < IR_Pin_Count; i++) {
    ir_weight[i] = 0;
  }

  uint64_t start = micros();
  while (micros() - start < /* TODO: integration time */) {
    for (int i = 0; i < IR_Pin_Count; i++) {
      int pin_state = /* TODO: digitalRead with active LOW */;
      if (/* TODO: active condition */) {
        ir_weight[i]++;
      }
    }
  }
  return true;
}

// ============ Setup ============
void setup() {
  Serial.begin(/* TODO: baud rate */);
  Serial0.begin(/* TODO: baud rate */);

  for (int i = 0; i < IR_Pin_Count; i++) {
    pinMode(ir_pins[i], /* TODO: INPUT_PULLUP */);
  }

  xTaskCreatePinnedToCore(/* TODO: Task1 */, "Task1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(/* TODO: Task2 */, "Task2", 10000, NULL, 1, NULL, 1);
}

// ============ Task1: IR Reading ============
void /* TODO: function name */(void *parameter) {
  while (1) {
    IR_reading();
    read_done_flag = true;
  }
}

// ============ Task2: Max + Serial0 ============
void /* TODO: function name */(void *parameter) {
  while (1) {
    while (!read_done_flag);
    read_done_flag = false;

    // Step 1: Find maximum
    int max_index = 0;
    for (int i = 1; i < IR_Pin_Count; i++) {
      if (ir_weight[i] > ir_weight[max_index]) {
        max_index = i;
      }
    }

    uint8_t max_value  = /* TODO: get weight */;

    // Step 2: Debug print
    if(/*TODO: if Serail is wokring, show debugging data*/){
      Serial.print("Max = ");
      Serial.print(/* TODO: value */);
      Serial.print(" @ degree ");
      Serial.println(/* TODO: degree */);
      delay(100);
    }
    // Step 3: Serial0 sending (with checksum)
    uint8_t checksum = 0;
    uint32_t send_data = /* TODO: pack data like in full example */;
    Serial0.write(0xAA);
    Serial0.write(send_data);
    Serial0.write(0xEE);
  }
}

void loop() {
  // Empty
}
