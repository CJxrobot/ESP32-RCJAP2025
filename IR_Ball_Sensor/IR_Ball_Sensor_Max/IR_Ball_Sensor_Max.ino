// Practice Template: Dual Core IR Sensor Max Finder + Serial0
//Port: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
//Pin : 05 38 07 08 10 11 39 13 15 41 17 42 35 01 36 03
//{5,38,7,8,10,11,39,13,15,41,17,42,35,01,36,03}
#define IR_Port_Count 10

TaskHandle_t Task1;
TaskHandle_t Task2;

uint16_t ir_port[IR_Port_Count] = {0,2,3,4,5,7,8,10,13,15};
uint8_t ir_pins[IR_Port_Count]  = {5,7,8,10,11,13,15,17,1,3};
uint16_t ir_weight[IR_Port_Count];

volatile bool read_done_flag = false;
volatile uint16_t max_value = 0;

void rgbLEDWrite(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
  rmt_data_t led_data[24];
  uint8_t pin = 38;
  rmtInit(pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
  // default WS2812B color order is G, R, B
  int color[3] = {green_val, red_val, blue_val};
  int i = 0;
  for (int col = 0; col < 3; col++) {
    for (int bit = 0; bit < 8; bit++) {
      if ((color[col] & (1 << (7 - bit)))) {
        // HIGH bit
        led_data[i].level0 = 1;     // T1H
        led_data[i].duration0 = 8;  // 0.8us
        led_data[i].level1 = 0;     // T1L
        led_data[i].duration1 = 4;  // 0.4us
      } else {
        // LOW bit
        led_data[i].level0 = 1;     // T0H
        led_data[i].duration0 = 4;  // 0.4us
        led_data[i].level1 = 0;     // T0L
        led_data[i].duration1 = 8;  // 0.8us
      }
      i++;
    }
  }
  rmtWrite(pin, led_data, RMT_SYMBOLS_OF(led_data), RMT_WAIT_FOR_EVER);
}

// ============ IR Reading ============
bool IR_reading() {
  for (uint8_t i = 0; i < IR_Port_Count; i++) {
    ir_weight[i] = 0;
  }

  uint64_t start = micros();
  while (micros() - start < 833) {
    for (uint8_t i = 0; i < IR_Port_Count; i++) {
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
  rgbLEDWrite(125,0,0);
  for (uint8_t i = 0; i < IR_Port_Count; i++) {
    pinMode(ir_pins[i], INPUT);
  }
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
}


// ============ Task1: IR Reading ============
void Task1code(void *parameter) {
  while (1) {
    read_done_flag = IR_reading();
  }
}

// ============ Task2: Max + Serial0 ============
void Task2code(void *parameter) {
  while (1) {
    while (!read_done_flag);
    read_done_flag = false;

    // Step 1: Find maximum
    int max_index = 0;
    for (uint8_t i = 0; i < IR_Port_Count; i++) {
      if (ir_weight[i] > ir_weight[max_index]) {
        max_index = i;
      }
    }
    uint8_t dis = 0;
    if (max_value > 0) {
      dis = (uint8_t)(16.0 * ir_weight[max_index] / (float)max_value);
    }
    // Step 3: pack and send with checksum
    uint8_t send_data = ((dis & 0xF0) >> 4) | (max_index & 0x0F);
    Serial0.write(0xAA);
    Serial0.write(send_data);
    Serial0.write(0xEE);
    // Step 2: Debug print
    if(Serial.available()){
      rgbLEDWrite(125,0,125);
      Serial.print("Max = ");
      Serial.print(max_index);
      Serial.print(" @ distance ");
      Serial.println(dis);
      delay(100);
    }
    if(max_index == 3 || max_index == 4){
      rgbLEDWrite(0,125,0);
    }
    else{
      rgbLEDWrite(125,0,0);
    }
    // Step 3: Serial0 sending (with checksum)
    
  }
}

void loop() {
  // Empty
}

