#include <Preferences.h>
#include <math.h>
#include <Arduino.h>

//Port: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
//Pin : 05 06 07 08 09 10 11 12 13 14 15 16 17 18 01 02 03 04
//{5,6,7,8,9,10,11,12,13,14,15,16,17,18,1,2,3,4};
#define LS_count 18
#define CAL_START_Command 0xAA
#define CAL_END_Command   0xEE

TaskHandle_t Task1;
TaskHandle_t Task2;

uint16_t max_ls[LS_count]; 
uint16_t avg_ls[LS_count];
uint16_t min_ls[LS_count];
uint16_t result[LS_count];

volatile bool read_done_flag = false;

uint8_t pin_config[LS_count] = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,1,2,3,4};
Preferences memory;

// Task handles
TaskHandle_t Task1;
TaskHandle_t Task2;

// Function prototypes
void Task1code(void * parameter);
void Task2code(void * parameter);
void scanning();
void save_avg();
uint32_t raw_data();

void rgbLEDWrite(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
  rmt_data_t led_data[24];
  rmtInit(38, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);
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
  rmtWrite(38, led_data, RMT_SYMBOLS_OF(led_data), RMT_WAIT_FOR_EVER);
}

// State machine for calibration
enum State {
  CALIBRATION_START,
  CALIBRATION_END,
  OPERATES_NORMALLY
};
State currentState = OPERATES_NORMALLY;

void setup() {
    Serial.begin(115200);
    Serial0.begin(115200);

    // Initialize sensor pins
    for (uint8_t i = 0; i < LS_count; i++) {
        pinMode(pin_config[i], INPUT);
        max_ls[i] = 0;
        min_ls[i] = 4095;
    }
    // Preferences init
    memory.begin("LS_avg", false);
    memory.getBytes("LS_avg", avg_ls, memory.getBytesLength("LS_avg"));

    // Create dual-core tasks
    xTaskCreatePinnedToCore(Task1code, "Task1", 4000, NULL, 1, &Task1, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 4000, NULL, 1, &Task2, 1);
}

void Task1code(void * parameter) {
  while(true){
    read_done_flag = false;

    // Read all sensors
    for(uint8_t i = 0; i < LS_count; i++){
      result[i] = analogRead(pin_config[i]);
    }

    read_done_flag = true;

    // UART0 commands for calibration
    if (Serial0.available() > 0) {
        uint8_t command = Serial0.read();
        if(command == CAL_START_Command) currentState = CALIBRATION_START;
        if(command == CAL_END_Command)   currentState = CALIBRATION_END;
    }
    if (Serial.available() > 0) {
        uint8_t command = Serial.read();
        if(command == CAL_START_Command) currentState = CALIBRATION_START;
        if(command == CAL_END_Command)   currentState = CALIBRATION_END;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void Task2code(void * parameter) {
  while(true){
    switch(currentState){
      case CALIBRATION_START:
        rgbLEDWrite(125,0,125);
        scanning();       // Update max/min
        break;
      case CALIBRATION_END:
        save_avg();       // Save averages to EEPROM
        currentState = OPERATES_NORMALLY;
        rgbLEDWrite(0,125,0);
        delay(500);
        rgbLEDWrite(0,0,0);
        delay(500);
        rgbLEDWrite(0,125,0);
        delay(500);
        break;
      case OPERATES_NORMALLY:
        // Send raw data over Serial0
        if(read_done_flag){
          uint32_t data = raw_data();
          uint8_t checksum = 0;
          if(data){
            rgbLEDWrite(125,0,0);
            Serial0.write(0xAA);
            Serial0.write(0xAA);
            for(uint8_t i = 0; i < 4; i++){
              uint8_t temp = (data >> (i*8)) & 0xFF;
              Serial0.write(temp);
              checksum += temp;
            }
            checksum &= 0xFF;
            Serial0.write(checksum);
            Serial0.write(0xEE);
          }
          if (Serial.available()) {
            Serial.print("ls_state");
            Serial.println(data, BIN);
            delay(100);
          }
          rgbLEDWrite(0,125,0);
        }
        break;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// Update max/min for calibration
void scanning(){
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    uint16_t reading = result[i];
    if(reading > max_ls[i]) max_ls[i] = reading;
    if(reading < min_ls[i]) min_ls[i] = reading;
  }
}

// Save average to EEPROM
void save_avg(){
  memory.clear();
  for(uint8_t i = 0; i < LS_count; i++){
    avg_ls[i] = (max_ls[i] + min_ls[i]) / 2;
  }
  memory.putBytes("LS_avg", avg_ls, sizeof(avg_ls)); 
}

// Convert sensor readings to raw bitmask
uint32_t raw_data(){
  uint32_t ls_state = 0;
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    if(result[i] > avg_ls[i]){
      ls_state |= (1UL << i);
    }
  }
  return ls_state;
}

void loop() {
  // Empty
}
