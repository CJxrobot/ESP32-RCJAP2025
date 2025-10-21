#include <Preferences.h>  // Non-volatile storage (like EEPROM) on ESP32
#include <math.h>
#include <Arduino.h>

// =============================================================
// Light Sensor Array (18 Sensors)
// =============================================================
#define LS_count 18

// Commands for calibration mode (sent over UART)
#define CAL_START_Command 0xAA
#define CAL_END_Command   0xEE

// Arrays for calibration and live sensor values
uint16_t max_ls[LS_count];
uint16_t avg_ls[LS_count];
uint16_t min_ls[LS_count];
uint16_t result[LS_count];

volatile bool read_done_flag = false;

// Sensor pin configuration
uint8_t pin_config[LS_count] = {
    5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 1, 2, 3, 4
};

// Preferences object (EEPROM)
Preferences memory;

// FreeRTOS task handles
TaskHandle_t Task1;
TaskHandle_t Task2;
uint32_t ls_state = 0;
bool send_data = false;

// ================================
// Function Prototypes
// ================================
void Task1code(void * parameter);
void Task2code(void * parameter);
void scanning();
void save_avg();
uint32_t raw_data();

// ================================
// RGB LED Control (WS2812B)
// ================================
void rgbLEDWrite(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
  rmt_data_t led_data[24];
  rmtInit(38, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);

  int color[3] = {red_val, green_val, blue_val};
  int i = 0;

  for (int col = 0; col < 3; col++) {
    for (int bit = 0; bit < 8; bit++) {
      if ((color[col] & (1 << (7 - bit)))) {
        led_data[i].level0 = 1; led_data[i].duration0 = 8;
        led_data[i].level1 = 0; led_data[i].duration1 = 4;
      } else {
        led_data[i].level0 = 1; led_data[i].duration0 = 4;
        led_data[i].level1 = 0; led_data[i].duration1 = 8;
      }
      i++;
    }
  }

  rmtWrite(38, led_data, RMT_SYMBOLS_OF(led_data), RMT_WAIT_FOR_EVER);
}

// ================================
// State Machine for Calibration
// ================================
enum State {
  CALIBRATION_START,
  CALIBRATION_END,
  OPERATES_NORMALLY
};
State currentState = OPERATES_NORMALLY;

// ================================
// Setup Function
// ================================
void setup() {
    Serial.begin(115200);   // Debug over USB
    Serial0.begin(115200);  // UART0 to external controller

    for (uint8_t i = 0; i < LS_count; i++) {
        pinMode(pin_config[i], INPUT);
        max_ls[i] = 0;
        min_ls[i] = 4095;
    }

    memory.begin("LS_avg", false);
    memory.getBytes("LS_avg", avg_ls, memory.getBytesLength("LS_avg"));

    xTaskCreatePinnedToCore(Task1code, "Task1", 4000, NULL, 1, &Task1, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 4000, NULL, 1, &Task2, 1);
}

// ================================
// Task1: Sensor Reading + Commands
// ================================
void Task1code(void * parameter) {
  while(true){
    read_done_flag = false;

    for(uint8_t i = 0; i < LS_count; i++){
      if(analogRead(pin_config[i]) > avg_ls[i]) {
        ls_state |= (1UL << i);
      }
    }
    read_done_flag = true;

    // UART0 commands
    if (Serial0.available() > 0) {
        uint8_t command = Serial0.read();
        if(command == CAL_START_Command) currentState = CALIBRATION_START;
        if(command == CAL_END_Command)   currentState = CALIBRATION_END;
        if(command == 0xdd) send_data = true;
    }

    // USB debug commands
    if (Serial.available() > 0) {
        uint8_t command = Serial.read();
        if(command == 'S') currentState = CALIBRATION_START;  // Start calibration
        if(command == 'E') currentState = CALIBRATION_END;    // End calibration
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// ================================
// Task2: Calibration + Data Sending
// ================================
void Task2code(void * parameter) {
  while(true){
    switch(currentState){
      case CALIBRATION_START:
        rgbLEDWrite(125,0,125); // Magenta
        scanning();
        break;

      case CALIBRATION_END:
        save_avg();
        currentState = OPERATES_NORMALLY;

        rgbLEDWrite(0,125,0); delay(500);
        rgbLEDWrite(0,0,0);   delay(500);
        rgbLEDWrite(0,125,0); delay(500);
        break;

      case OPERATES_NORMALLY:
        while(!read_done_flag);
        // Send data frame: [0xAA 0xAA][4 bytes][checksum][0xEE]
        if(send_data){
          Serial0.write(0xAA);
          uint8_t checksum = 0;
          for(uint8_t i = 0; i < 4; i++){
            uint8_t temp = (ls_state >> (i*8)) & 0xFF;
            Serial0.write(temp);   // send 1 byte
            checksum += temp;
          }
          checksum &= 0xFF;  // keep in 8 bits
          Serial0.write(checksum);
          Serial0.write(0xEE);
          send_data = false;
        }
        if(ls_state == 0b111111111111111111){
          rgbLEDWrite(0,0,0);
        }
        else{
          rgbLEDWrite(125,0,0);
        }
        ls_state = 0;
        // Debug print
        if (Serial.available()) {
          Serial.print("ls_state: ");
          Serial.println(ls_state, BIN);
          delay(100);
        }
        break;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// ================================
// Helper Functions
// ================================
void scanning(){
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    uint16_t reading = result[i];
    if (reading > max_ls[i]) max_ls[i] = reading;
    if (reading < min_ls[i]) min_ls[i] = reading;
  }
}

void save_avg(){
  memory.clear();
  for(uint8_t i = 0; i < LS_count; i++){
    avg_ls[i] = (max_ls[i] + min_ls[i]) / 2;
  }
  memory.putBytes("LS_avg", avg_ls, sizeof(avg_ls));
}

uint32_t raw_data(){

  while(!read_done_flag);
  
  return ls_state;
}

// ================================
// Main Loop (empty)
// ================================
void loop() {
  // empty — all handled by FreeRTOS
}
