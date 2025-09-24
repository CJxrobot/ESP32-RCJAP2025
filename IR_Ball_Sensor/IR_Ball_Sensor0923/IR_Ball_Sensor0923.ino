#define PI 3.14159265

#define SD_IR_Pin_Count 13
uint8_t sd_ir_pins[SD_IR_Pin_Count] = {15,16,17,18,1,2,3,5,6,7,11,12,13};
//uint16_t sd_ir_degree[SD_IR_Pin_Count] = {40,60,80,90,100,120,140,190,220,250,290,320,340};
uint16_t sd_ir_degree[SD_IR_Pin_Count] = {140,120,100,90,80,60,40,340,320,290,250,220,190};
uint16_t sd_ir_weight[SD_IR_Pin_Count];
float sd_cos_deg[SD_IR_Pin_Count], sd_sin_deg[SD_IR_Pin_Count];
/*
#define SD_IR_Pin_Count 13
uint8_t sd_ir_pins[SD_IR_Pin_Count] = {15,16,17,18,1,2,3,5,6,7,11,12,13};
uint16_t sd_ir_degree[SD_IR_Pin_Count] = {340,320,290,250,220,190,140,120,100,90,80,60,40};
uint16_t sd_ir_weight[SD_IR_Pin_Count];
float sd_cos_deg[SD_IR_Pin_Count], sd_sin_deg[SD_IR_Pin_Count];
*/

//#define LD_IR_Pin_Count 10
//uint8_t ld_ir_pins[LD_IR_Pin_Count] = {40,41,42,35,36,37,38,8,10,39};
//uint16_t ld_ir_degree[LD_IR_Pin_Count] = {0,45,85,95,135,180,225,265,275,315};
#define LD_IR_Pin_Count 8
uint8_t ld_ir_pins[LD_IR_Pin_Count] = {41,42,35,36,38,8,10,39};
//uint16_t ld_ir_degree[LD_IR_Pin_Count] = {45,85,95,135,225,265,275,315};
uint16_t ld_ir_degree[LD_IR_Pin_Count] = {135,95,85,45,315,275,265,225};




/*//flip
#define LD_IR_Pin_Count 10
uint8_t ld_ir_pins[LD_IR_Pin_Count] = {40,41,42,35,36,37,38,8,10,39};
uint16_t ld_ir_degree[LD_IR_Pin_Count] = {315,275,265,225,180,135,95,85,45,0};
*/

/*
#define LD_IR_Pin_Count 6
uint8_t ld_ir_pins[LD_IR_Pin_Count] = {40,42,35,37,8,10};
uint16_t ld_ir_degree[LD_IR_Pin_Count] = {0,85,95,180,265,275};
*/

/*//flip
#define LD_IR_Pin_Count 6
uint8_t ld_ir_pins[LD_IR_Pin_Count] = {40,42,35,37,8,10};
uint16_t ld_ir_degree[LD_IR_Pin_Count] = {275,265,180,95,85,0};
*/
uint16_t ld_ir_weight[LD_IR_Pin_Count];
float ld_cos_deg[LD_IR_Pin_Count], ld_sin_deg[LD_IR_Pin_Count];
float prior_ball_degree;
uint16_t max_sum = 0;

bool sd_read_done_flag = false;
bool ld_read_done_flag = false;

void Task1code(void *parameter);
void Task2code(void *parameter);

//#define DEBUG
//#define OFFSET

bool shortd_IR() {
    for (uint8_t i = 0; i < SD_IR_Pin_Count; i++) {
        sd_ir_weight[i] = 0;
    }
    for(uint16_t c = 0; c < 500; c++){
        for (uint8_t i = 0; i < SD_IR_Pin_Count; i++) {
            uint16_t read_value = 4095 - analogRead(sd_ir_pins[i]);
            if(read_value > sd_ir_weight[i]){
              sd_ir_weight[i] = read_value;
            }
        }
    }
    
    return true;
}

bool longd_IR() {
    uint64_t time = micros();
    for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
        ld_ir_weight[i] = 0;
    }
    while (micros() - time < 833) {  // Wait for the required time period
        uint32_t gpio_in  = REG_READ(GPIO_IN_REG);
        uint32_t gpio_in1 = REG_READ(GPIO_IN1_REG);
        for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
            uint8_t pin = ld_ir_pins[i];
            bool pin_state;
            if (pin < 32) {
              pin_state = !((gpio_in >> pin) & 0x1);  // active LOW
            } else {
              pin_state = !((gpio_in1 >> (pin - 32)) & 0x1);  // active LOW
            }
            if (pin_state) {
              ld_ir_weight[i]++;
            }
        }
    }
    return true;
}

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
        pinMode(ld_ir_pins[i], INPUT_PULLUP);
        ld_cos_deg[i] = cos(ld_ir_degree[i] * PI / 180.0);
        ld_sin_deg[i] = sin(ld_ir_degree[i] * PI / 180.0);
    }

    for (uint8_t i = 0; i < SD_IR_Pin_Count; i++) {
        pinMode(sd_ir_pins[i], INPUT_PULLUP);
        // Uncomment if you want to use angle cos/sin later
        // sd_cos_deg[i] = cos(sd_ir_degree[i] * PI / 180.0);
        // sd_sin_deg[i] = sin(sd_ir_degree[i] * PI / 180.0);
    }
    // Warm-up reads for long-distance IR sensors
    for (uint8_t i = 0; i < 100; i++) {
        longd_IR();
        uint16_t sum = 0;
        for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
            sum += ld_ir_weight[i];
        }
        if (sum > max_sum) {
            max_sum = sum;
        }
    }
    delay(100);
    // Create tasks
    xTaskCreatePinnedToCore(
        Task1code,   // Function to implement the task
        "Task1",     // Name of the task
        10000,       // Stack size in words
        NULL,        // Task input parameter
        1,           // Priority of the task
        NULL,        // Task handle
        0);          // Core where the task should run

    xTaskCreatePinnedToCore(
        Task2code,   // Function to implement the task
        "Task2",     // Name of the task
        10000,       // Stack size in words
        NULL,        // Task input parameter
        1,           // Priority of the task
        NULL,        // Task handle
        1);          // Core where the task should run
}

void Task1code(void *parameter) {
  while(1){
    sd_read_done_flag = false;
    ld_read_done_flag = false;

    while (!shortd_IR());  // Read short distance IR sensors
    sd_read_done_flag = true;

    while (!longd_IR());   // Read long distance IR sensors
    ld_read_done_flag = true;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void Task2code(void *parameter) {
  while(1){
    uint16_t ball_direction = 999;
    uint16_t ball_distance = 0;
    float sum = 0;
    while (!ld_read_done_flag);  // Wait for long distance data
    for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
      sum += ld_ir_weight[i];
    }
    if (sum > max_sum) {
      max_sum = sum;
    }
    ball_distance = (sum / max_sum) * 255;  // You may need to replace this with the actual calculation
    if(ball_distance > 255){
      ball_distance = 255;
    }
    float alpha = (ball_distance / 255.0) * 0.5;
    //Serial.printf("distance:%d; alpha:%f\n", ball_distance, alpha);
    //c = 0;
    // If no ball is detected, check long distance sensor
    if (alpha < 0.25) {
      #ifdef DEBUG
        Serial.println("long");
      #endif
        if(sum != 0){
          /*
          uint8_t power = (int)(1.0/alpha);
          for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
            for(uint8_t p = 0; p < power; p++){
              ld_ir_weight[i] = ld_ir_weight[i] * ld_ir_weight[i];
            }
          }*/
          sum = 0;
          // Vector sum for long distance sensor readings
          float x = 0, y = 0;
          for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
              #ifdef DEBUG
                  Serial.printf("Sensor %d| Port %d | Degree %d = %d\n", i, ld_ir_pins[i], ld_ir_degree[i], ld_ir_weight[i]);
              #endif
              //x += ld_ir_weight[i] * ld_cos_deg[i];
              //y += ld_ir_weight[i] * ld_sin_deg[i];
              //sum += ld_ir_weight[i];
              float weight = pow((float)ld_ir_weight[i], 1+alpha);  // try 1.2–2.0
              x += weight * ld_cos_deg[i];
              y += weight * ld_sin_deg[i];
              sum += weight;
          }
          x = x / (sum * 1.0);
          y = y / (sum * 1.0);          
          ball_direction = (int)(fmod((atan2(y, x) * 180 / PI) + 360, 360));
          ball_direction = (1 - alpha) * ball_direction + prior_ball_degree * alpha;
          prior_ball_degree = ball_direction;
        }
        else{
          prior_ball_degree = 0;
          ball_direction = 999;
          ball_distance = 999;  // You may need to replace this with the actual calculation
        }
    }
    else {
        // Check whether short distance sensor senses the ball or not
        while (!sd_read_done_flag);  // Wait for short distance data
        #ifdef DEBUG
          Serial.println("short");
        #endif
        uint16_t max_value = 0;
        uint8_t max_index = 255;

        // Find the maximum sensor reading in the short distance sensors
        for (uint8_t i = 0; i < SD_IR_Pin_Count; i++) {
            if (sd_ir_weight[i] > max_value) {
                max_value = sd_ir_weight[i];
                max_index = i;
            }
            #ifdef DEBUG
                Serial.printf("Sensor %d Port %d = %d\n", i, sd_ir_pins[i], sd_ir_weight[i]);
            #endif
        }
        // Vector sum for short distance sensor readings (Optional)
        /*
          float x = 0, y = 0, sum = 0;
          for (uint8_t i = 0; i < LD_IR_Pin_Count; i++) {
              x += sd_ir_weight[i] * sd_cos_deg[i];
              y += sd_ir_weight[i] * sd_sin_deg[i];
              sum += sd_ir_weight[i];
          }

          x = x / sum;
          y = y / sum;
          ball_direction = (int)(fmod((atan2(y, x) * 180 / PI) + 360, 360));
          ball_distance = (int)(sum * 0.1);
        */
        ball_direction = sd_ir_degree[max_index];
        //ball_distance = (int)((sd_ir_weight[max_index] / 4095.0) * 999);
#ifdef DEBUG
    Serial.printf("\nmax_index:%d\n", max_index);
#endif
    }
#ifdef DEBUG
    Serial.printf("\nball_dir:%d, ball_distance:%d\n", ball_direction, ball_distance);
#endif
    // Send data over serial
    Serial0.write(0xAA);
    Serial0.write(0xAA);
    uint8_t checksum = 0x00;
    uint16_t ball_deg = (uint16_t)(int)ball_direction;
    uint32_t send_data = (ball_deg & 0xFF) | ((ball_deg >> 8) & 0xFF) << 8 | (ball_distance & 0xFF) << 16 | ((ball_distance >> 8) & 0xFF) << 24;
    for (uint8_t i = 0; i < 4; i++) {
      uint8_t temp = (send_data >> (i * 8)) & 0xFF;
#ifdef DEBUG
      Serial.println(temp);
#endif
      Serial0.write(temp);
      checksum += temp;
    }
    checksum &= 0xFF;
    Serial0.write(checksum);
    Serial0.write(0xEE);
#ifdef DEBUG
    delay(200);
#endif
/*
    Serial.print(0);
    Serial.print(",");
    Serial.print(ball_deg);
    Serial.print(",");
    Serial.print(360);
    Serial.printf("\n");


    Serial.print(0);
    Serial.print(",");
    Serial.print(ball_);
    Serial.print(",");
    Serial.print(255);
    Serial.printf("\n");
    delay(10);
*/
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void loop() {
    // Empty loop because tasks are being handled in Task1 and Task2
}
