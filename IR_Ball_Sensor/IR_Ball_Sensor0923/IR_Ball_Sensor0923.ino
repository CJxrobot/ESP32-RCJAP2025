#define PI 3.14159265
#define IR_Pin_Count 8

uint8_t ir_pins[IR_Pin_Count] = {41,42,35,36,38,8,10,39};
uint16_t ir_degree[IR_Pin_Count] = {135,95,85,45,315,275,265,225};

uint16_t ir_weight[IR_Pin_Count];
float cos_deg[IR_Pin_Count], sin_deg[IR_Pin_Count];
uint16_t max_sum = 0;

void Task1code(void *parameter);
void Task2code(void *parameter);

bool ball_detected = false;
volatile bool read_done_flag = false;   // used between tasks

bool IR_reading() {
  uint64_t time = micros();
  for (uint8_t i = 0; i < IR_Pin_Count; i++) {
      ir_weight[i] = 0;
  }
  while (micros() - time < 833) {  // integration window
    uint32_t gpio_in  = REG_READ(GPIO_IN_REG);
    uint32_t gpio_in1 = REG_READ(GPIO_IN1_REG);
    for (uint8_t i = 0; i < IR_Pin_Count; i++) {
      uint8_t pin = ir_pins[i];
      bool pin_state;
      if (pin < 32) {
        pin_state = !((gpio_in >> pin) & 0x1);  // active LOW
      } else {
        pin_state = !((gpio_in1 >> (pin - 32)) & 0x1);  // active LOW
      }
      if (pin_state) {
        ir_weight[i]++;
      }
    }
  }
  return true;
}

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    for (uint8_t i = 0; i < IR_Pin_Count; i++) {
        pinMode(ir_pins[i], INPUT_PULLUP);
        cos_deg[i] = cos(ir_degree[i] * PI / 180.0);
        sin_deg[i] = sin(ir_degree[i] * PI / 180.0);
    }

    // Warm-up reads for IR sensors
    for (uint8_t i = 0; i < 100; i++) {
        IR_reading();
        uint16_t sum = 0;
        for (uint8_t j = 0; j < IR_Pin_Count; j++) {
            sum += ir_weight[j];
        }
        if (sum > max_sum) {
            max_sum = sum;
        }
    }
    delay(100);

    // Create tasks
    xTaskCreatePinnedToCore(
        Task1code,
        "Task1",
        10000,
        NULL,
        1,
        NULL,
        0);

    xTaskCreatePinnedToCore(
        Task2code,
        "Task2",
        10000,
        NULL,
        1,
        NULL,
        1);
}

void Task1code(void *parameter) {
  while(1){
    IR_reading();          // just call IR_reading() instead of longd_IR()
    read_done_flag = true;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void Task2code(void *parameter) {
  while(1){
    uint16_t ball_direction = 999;
    uint16_t ball_distance = 0;
    float sum = 0;
    while (!read_done_flag);  // Wait for data
    read_done_flag = false;   // reset flag

    for (uint8_t i = 0; i < IR_Pin_Count; i++) {
      sum += ir_weight[i];
    }
    if (sum > max_sum) {
      max_sum = sum;
    }
    ball_distance = (sum / max_sum) * 255;
    if(ball_distance > 255){
      ball_distance = 255;
    }

    if(sum != 0){
      ball_detected = true;
      sum = 0;
      float x = 0, y = 0;
      for (uint8_t i = 0; i < IR_Pin_Count; i++) {
          x += ir_weight[i] * cos_deg[i];
          y += ir_weight[i] * sin_deg[i];
          sum += ir_weight[i];
      }
      x /= sum;
      y /= sum;
      ball_direction = (int)(fmod((atan2(y, x) * 180 / PI) + 360, 360));
    }
    else {
      ball_detected = false;
      ball_direction = 999;
      ball_distance = 999;
    }

    if(ball_detected){
      // Send data over serial
      uint8_t checksum = 0x00;
      uint16_t ball_deg = (uint16_t)(int)ball_direction;
      uint32_t send_data = (ball_deg & 0xFF) | ((ball_deg >> 8) & 0xFF) << 8 | (ball_distance & 0xFF) << 16 | ((ball_distance >> 8) & 0xFF) << 24;
      Serial0.write(0xAA);
      Serial0.write(0xAA);                     
      for (uint8_t i = 0; i < 4; i++) {
        uint8_t temp = (send_data >> (i * 8)) & 0xFF;
        Serial0.write(temp);
        checksum += temp;
      }
      checksum &= 0xFF;
      Serial0.write(checksum);
      Serial0.write(0xEE);
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Empty, tasks run independently
}