#include <Preferences.h>
#include <math.h>
#include <Arduino.h>

#define LS_count 18
#define CAL_START_Command 0xAA
#define CAL_END_Command   0xEE

bool read_done_flag = false;
uint16_t max_ls[LS_count]; 
uint16_t avg_ls[LS_count];
uint16_t min_ls[LS_count];
uint16_t result[LS_count];

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

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void Task2code(void * parameter) {
  while(true){
    switch(currentState){
      case CALIBRATION_START:
        scanning();       // Update max/min
        break;
      case CALIBRATION_END:
        save_avg();       // Save averages to EEPROM
        currentState = OPERATES_NORMALLY;
        break;
      case OPERATES_NORMALLY:
        // Send raw data over Serial0
        if(read_done_flag){
          uint32_t data = raw_data();
          uint8_t checksum = 0;
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
        break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
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

void loop(){
  // Tasks handle everything
}
