// Practice Template: Dual Core IR Sensor Max Finder + Serial0
// Students: Fill in the TODO parts

#define IR_Pin_Count 10

uint8_t ir_pins[IR_Pin_Count]   = { /* TODO: pin numbers */ };
uint16_t ir_port[IR_Pin_Count] = { /* TODO: degrees */ };
uint16_t ir_weight[IR_Pin_Count];

volatile bool read_done_flag = false;
volatile uint16_t max_value = 0;


// ============ IR Reading ============
bool IR_reading() {
  for (uint8_t i = 0; i < IR_Pin_Count; i++) {
    ir_weight[i] = 0;
  }

  uint64_t start = micros();
  while (micros() - start < 833) {
    for (uint8_t i = 0; i < IR_Pin_Count; i++) {
      uint8_t pin_state = digitalRead(ir_pins[i]);
      if (!pin_state) {
        ir_weight[i]++;
      }
    }
  }
  return true;
}

// ============ Setup ============
void setup() {
  Serial.begin(115200);
  Serial0.begin(115200);
  for (uint8_t i = 0; i < IR_Pin_Count; i++) {
    pinMode(ir_pins[i], INPUT);
  }

  xTaskCreatePinnedToCore(TaskRead, "Task1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskSend, "Task2", 10000, NULL, 1, NULL, 1);
}

// ============ Task1: IR Reading ============
void TaskRead(void *parameter) {
  while (1) {
    IR_reading();
    read_done_flag = true;
  }
}

// ============ Task2: Max + Serial0 ============
void TaskSend(void *parameter) {
  while (1) {
    while (!read_done_flag);
    read_done_flag = false;

    // Step 1: Find maximum
    int max_index = 0;
    max_value = ir_weight[i];
    for (uint8_t i = 1; i < IR_Pin_Count; i++) {
      if (ir_weight[i] > ir_weight[max_index]) {
        max_index = i;
      }
    }
    if(ir_weight[max_index] > max_value){
      max_value = ir_weight[max_index];
    }
    uint8_t dis  = 16 * (ir_weight[max_index] / max_value);

    // Step 2: Debug print
    if(Serial.available()){
      //LED Green
      Serial.print("Max = ");
      Serial.print(/* TODO: value */);
      Serial.print(" @ distance ");
      Serial.println(/* TODO: distance */);
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

