#include <Preferences.h>
#include <math.h>
#define RO_MODE true
#define RW_MODE false
#define CAL_START_Command 0xaa
#define CAL_END_Command 0xee
#define OPERATE_Command 0x10
#define DUMP_Command 0xdd
#define SWM_Command 0xad

#define LS_count 18
#define radius 5
#define CONVERSIONS_PER_PIN 
//#define DEBUG

bool read_done_flag = false;
bool offense = true;
uint16_t max_ls[LS_count]; 
uint16_t avg_ls[LS_count];
uint16_t min_ls[LS_count];
uint16_t result[LS_count];
//uint8_t pin_config[LS_count] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
uint8_t pin_config[LS_count] = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,1,2,3,4};
// Sensor angle configuration: 10°, 30°, ..., 350°
const float SENSOR_ANGLES[LS_count] = {10, 30, 50, 70, 90, 110, 130, 150, 170,190, 210, 230, 250, 270, 290, 310, 330, 350};

float cos_deg[LS_count], sin_deg[LS_count];
Preferences memory;

// Task handles
TaskHandle_t Task1;
TaskHandle_t Task2;

// Function prototypes
void Task1code(void * parameter);
void Task2code(void * parameter);
void scanning();
void save_avg();
uint16_t raw_data();

// Define states
enum State {
  CALIBRATION_START,
  CALIBRATION_END,
  OPERATES_NORMALLY,
  DUMP_DATA
};

State currentState = OPERATES_NORMALLY;

void rgbLEDWrite(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
  rmt_data_t led_data[24];
  uint8_t pin = 38;
  rmtInit(pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);

  // default WS2812B color order is R, G, B
  //int color[3] = {green_val, red_val, blue_val};
  int color[3] = {red_val, green_val, blue_val};


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

void setup() {
    // Start serial communication
  Serial.begin(115200);
  //Serial1.begin(115200);
  Serial0.begin(115200);
  Serial.println("init_done");
  // Create tasks
  xTaskCreatePinnedToCore(
    Task1code,   // Function to implement the task
    "Task1",     // Name of the task
    10000,       // Stack size in words
    NULL,        // Task input parameter
    1,           // Priority of the task
    &Task1,      // Task handle
    0);          // Core where the task should run

  xTaskCreatePinnedToCore(
    Task2code,   // Function to implement the task
    "Task2",     // Name of the task
    10000,       // Stack size in words
    NULL,        // Task input parameter
    1,           // Priority of the task
    &Task2,      // Task handle
    1);          // Core where the task should run
  
}

// Task1: Core 2 - Read commands from UART
void Task1code(void * parameter) {
  while(true){
    read_done_flag = false;
    for(uint8_t i = 0; i < LS_count; i++){
      result[i] = analogRead(pin_config[i]);
    }
    read_done_flag = true;
  #ifdef DEBUG
    if (Serial.available() > 0) {
      uint8_t command = Serial.read();
  #else
    if (Serial0.available() > 0) {
      uint8_t command = Serial0.read();
  #endif
      if (command) {
        Serial0.println("R");  // Send 'R' upon receiving a command
        switch (command){
          case SWM_Command:
            offense = (~offense);
            break;
          case CAL_START_Command:
            #ifdef DEBUG
              Serial.println("Calibration started");
            #endif
            currentState = CALIBRATION_START;
            break;

          case CAL_END_Command:
            if(currentState == CALIBRATION_START){
              #ifdef DEBUG
                Serial.println("Calibration ended");
              #endif
              currentState = CALIBRATION_END;
            }
            break;
          case OPERATE_Command:
            #ifdef DEBUG  
              Serial.println("Operating normally");
            #endif
            currentState = OPERATES_NORMALLY;
            break;
          case DUMP_Command:
            #ifdef DEBUG
              Serial.println("Dump");
            #endif
            currentState = DUMP_DATA;
            break;
          default:
            break;
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Task2: Core 1 - Switch operation state based on command
void Task2code(void * parameter) {
  //Pin init
  for(uint8_t i = 0; i < LS_count; i++){
    max_ls[i] = 0;
    min_ls[i] = 4095;
    float temp = 3.145926 * SENSOR_ANGLES[i] / 180; 
    cos_deg[i] = cos(temp);
    sin_deg[i] = sin(temp);
  }
  uint8_t checksum;
  //Memory function init
  memory.begin("LS_avg", RW_MODE);

  delay(100);
  memory.getBytes("LS_avg", avg_ls, memory.getBytesLength("LS_avg"));
  rgbLEDWrite(64,0,64);
  while (true) {
    //Serial.println("core1");
    uint32_t ls_state = 0b00000000000000000000000000000000;
    //Serial.println(read_done_flag);
    #ifdef DEBUG
      if(read_done_flag){
        for(uint8_t i = 0; i < LS_count; i++){
          uint16_t reading = result[i];
          
            Serial.printf("%d = ", i);
            Serial.print(reading);
            Serial.print(",");
            Serial.println(avg_ls[i]);
          if(reading > avg_ls[i]){
            ls_state |= (0b01 << i);
          }
        }
      }
    #endif
    switch(currentState){
      case OPERATES_NORMALLY:
        checksum = 0x00;
        uint32_t ls_final_data;
        //ls_final_data = rmc_cal();
        ls_final_data = calculate_line_tracking();
        #ifdef DEBUG
          Serial.printf("angle: %d°, size: %d°, ls_state: %x\n", 10 * (ls_final_data & 0x3F), 10 * ((ls_final_data >> 6) & 0x3F), (ls_final_data>> 14) & 0x3FFFF);
          Serial.print("final data: 0b");
          Serial.println(ls_final_data, BIN);
        #endif
        Serial0.flush();
        Serial0.write(0xAA);
        Serial0.write(0xAA);
        for(uint8_t i = 0; i < 4; i++){
          uint8_t temp = (ls_final_data >> (i * 8)) & 0xFF;
          #ifdef DEBUG
            Serial.println(temp);
          #endif
          Serial0.write(temp);
          checksum += temp;
        }
        checksum &= 0xFF;
        Serial0.write(checksum);
        Serial0.write(0xEE);
        //Serial.println("data");
        break;
      case DUMP_DATA:
        #ifdef DEBUG
          Serial.println("DUMP...");
        #endif
        delay(100);
        Serial0.print("$");
        for(uint8_t i = 0; i < LS_count; i++){
          Serial0.print(avg_ls[i]); Serial.print(", ");
        }
        Serial0.println("&");
        currentState = OPERATES_NORMALLY;
        break;
      case CALIBRATION_START:
        #ifdef DEBUG
          Serial.println("Calibration in progress...");
        #endif
        rgbLEDWrite(0,0,125);
        while (currentState != CALIBRATION_END) {
            scanning();
            vTaskDelay(100 / portTICK_PERIOD_MS); // Add delay to avoid blocking indefinitely
        }
        save_avg();
        #ifdef DEBUG
          Serial.println("Calibration completed.");
        #endif
        rgbLEDWrite(0,125,0);
        delay(1000);
        rgbLEDWrite(0,0,0);
        delay(1000);
        rgbLEDWrite(0,125,0);
        delay(1000);
        rgbLEDWrite(0,0,0);
        delay(1000);
        rgbLEDWrite(64,0,64);
        currentState = OPERATES_NORMALLY;
        break;
      default:
        break;
    }
    #ifdef DEBUG
      Serial.println("core1 END");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    #else
      vTaskDelay(10 / portTICK_PERIOD_MS);
    #endif
  }
}

void scanning(){
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    uint16_t reading = result[i];
    if(max_ls[i] < reading) {
      max_ls[i] = reading;
    }
    if(min_ls[i] > reading) {
      min_ls[i] = reading;
    }
  }
}

void save_avg(){
  memory.clear();
  for(uint8_t i = 0; i < LS_count; i++){
    avg_ls[i] = (max_ls[i] + min_ls[i]) / 2;
    #ifdef DEBUG
    Serial.print(i);
    Serial.print(": ");
    Serial.print(max_ls[i]);
    Serial.print(", ");
    Serial.println(min_ls[i]);
    #endif
  }
  memory.putBytes("LS_avg", avg_ls, sizeof(avg_ls)); 
  #ifdef DEBUG
  Serial.println("done");
  #endif
}

uint32_t raw_data(){
  uint32_t ls_state = 0b00000000000000000000000000000000;
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    #ifdef DEBUG
      Serial.print(result[i]);
      Serial.print(",");
      Serial.println(avg_ls[i]);
    #endif
    if(result[i] > avg_ls[i]){
      ls_state |= (0b01 << i);
    }
  }
  #ifdef DEBUG
    String binStr = "";
    for (int i = 31; i >= 0; i--) {
      binStr += (ls_state & (1 << i)) ? '1' : '0';  // Check each bit
    }
    Serial.println(binStr);  // Print the binary string
  #endif
  return ls_state;
}

void loop(){
  ;
}
