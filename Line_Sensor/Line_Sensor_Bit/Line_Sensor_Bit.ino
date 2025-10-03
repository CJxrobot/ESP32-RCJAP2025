#include <Preferences.h>  // Non-volatile storage (like EEPROM) on ESP32
#include <math.h>
#include <Arduino.h>

// =============================================================
// Light Sensor Array (18 Sensors)
// =============================================================
// We are using 18 analog light sensors. Each sensor is connected
// to a physical GPIO pin on the ESP32. The mapping is shown below.
//
// Port index : 00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15   16   17
// Physical pin: 05   06   07   08   09   10   11   12   13   14   15   16   17   18   01   02   03   04
//
// Port index is just an internal number (array index 0–17).
// Physical pin is the actual ESP32 GPIO number used for that sensor.
//
#define LS_count 18

// Commands for calibration mode (sent over UART)
#define CAL_START_Command 0xAA
#define CAL_END_Command   0xEE

// Arrays for calibration and live sensor values
uint16_t max_ls[LS_count];   // Maximum observed value during calibration
uint16_t avg_ls[LS_count];   // Average threshold (used for dark/bright decision)
uint16_t min_ls[LS_count];   // Minimum observed value during calibration
uint16_t result[LS_count];   // Current live reading of sensors

// Flag: true = a full scan of sensors is completed
volatile bool read_done_flag = false;

// Sensor pin configuration (matches the table above)
uint8_t pin_config[LS_count] = {
    5,   // Sensor 0 → GPIO 5
    6,   // Sensor 1 → GPIO 6
    7,   // Sensor 2 → GPIO 7
    8,   // Sensor 3 → GPIO 8
    9,   // Sensor 4 → GPIO 9
    10,  // Sensor 5 → GPIO 10
    11,  // Sensor 6 → GPIO 11
    12,  // Sensor 7 → GPIO 12
    13,  // Sensor 8 → GPIO 13
    14,  // Sensor 9 → GPIO 14
    15,  // Sensor 10 → GPIO 15
    16,  // Sensor 11 → GPIO 16
    17,  // Sensor 12 → GPIO 17
    18,  // Sensor 13 → GPIO 18
    1,   // Sensor 14 → GPIO 1
    2,   // Sensor 15 → GPIO 2
    3,   // Sensor 16 → GPIO 3
    4    // Sensor 17 → GPIO 4
};

// Preferences object = EEPROM-like storage for ESP32
Preferences memory;

// FreeRTOS task handles
TaskHandle_t Task1;
TaskHandle_t Task2;

// ================================
// Function Prototypes
// ================================
void Task1code(void * parameter);  // Reads sensors + handles commands
void Task2code(void * parameter);  // Handles calibration + data sending
void scanning();                   // Updates max/min for calibration
void save_avg();                   // Saves avg threshold into EEPROM
uint32_t raw_data();                // Packs sensor data into 32-bit bitmask

// ================================
// RGB LED Control (WS2812B)
// ================================
// This LED gives real-time feedback about system state
// - Magenta = calibration ongoing
// - Green   = normal operation
// - Red     = sending data
void rgbLEDWrite(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
  rmt_data_t led_data[24];  // 24 bits needed (8 bits each for G, R, B)
  rmtInit(38, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000);

  // WS2812B uses GRB order
  int color[3] = {green_val, red_val, blue_val};
  int i = 0;

  // Encode the color into RMT signal pulses
  for (int col = 0; col < 3; col++) {
    for (int bit = 0; bit < 8; bit++) {
      if ((color[col] & (1 << (7 - bit)))) {
        // HIGH-long, LOW-short for bit = 1
        led_data[i].level0 = 1; led_data[i].duration0 = 8;
        led_data[i].level1 = 0; led_data[i].duration1 = 4;
      } else {
        // HIGH-short, LOW-long for bit = 0
        led_data[i].level0 = 1; led_data[i].duration0 = 4;
        led_data[i].level1 = 0; led_data[i].duration1 = 8;
      }
      i++;
    }
  }

  // Send encoded color data to LED
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

    // Initialize all sensor pins as analog inputs
    for (uint8_t i = 0; i < LS_count; i++) {
        pinMode(pin_config[i], INPUT);
        max_ls[i] = 0;       // start with lowest
        min_ls[i] = 4095;    // start with highest (12-bit ADC max)
    }

    // Load saved calibration thresholds (if available) to the onboard EEPROM
    memory.begin("LS_avg", false);
    memory.getBytes("LS_avg", avg_ls, memory.getBytesLength("LS_avg"));

    // Create two FreeRTOS tasks (Task1 on Core 0, Task2 on Core 1)
    xTaskCreatePinnedToCore(Task1code, "Task1", 4000, NULL, 1, &Task1, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 4000, NULL, 1, &Task2, 1);
}

// ================================
// Task1: Sensor Reading + Commands
// ================================
// Core 0: reads sensors and listens for calibration commands
void Task1code(void * parameter) {
  while(true){
    read_done_flag = false;

    // Read all 18 sensors into result[]
    for(uint8_t i = 0; i < LS_count; i++){
      result[i] = analogRead(pin_config[i]);  // each value = 0–4095
    }

    read_done_flag = true;

    // Listen for calibration commands on Serial0
    if (Serial0.available() > 0) {
        uint8_t command = Serial0.read();
        if(command == CAL_START_Command) currentState = CALIBRATION_START;
        if(command == CAL_END_Command)   currentState = CALIBRATION_END;
    }

    // Debug commands from USB Serial
    if (Serial.available() > 0) {
        uint8_t command = Serial.read();
        if(command == /*ToDo*/) currentState = CALIBRATION_START;
        if(command == /*ToDo*/)   currentState = CALIBRATION_END;
    }

    vTaskDelay(1 / portTICK_PERIOD_MS); // tiny delay to yield CPU
  }
}

// ================================
// Task2: Calibration + Data Sending
// ================================
// Core 1: handles calibration and sends sensor states
void Task2code(void * parameter) {
  while(true){
    switch(currentState){
      case CALIBRATION_START:
        // Show magenta during calibration
        rgbLEDWrite(125,0,125);
        scanning(); // update min/max values
        break;

      case CALIBRATION_END:
        // Save average thresholds to EEPROM
        save_avg();
        currentState = OPERATES_NORMALLY;

        // Blink green to show calibration success
        rgbLEDWrite(0,125,0); delay(500);
        rgbLEDWrite(0,0,0);   delay(500);
        rgbLEDWrite(0,125,0); delay(500);
        break;

      case OPERATES_NORMALLY:
        // Normal mode = pack sensor data + send via UART
        if(read_done_flag){
          uint32_t data = raw_data();  // get 18-bit sensor state
          uint8_t checksum = 0;

          if(data){
            // Red flash = sending data
            /*
              ToDo:
              if data exist / white line touched, turn the red led on using the function rgbLEDWrite()
            */

            // Send frame: [0xAA 0xAA] + [4 bytes data] + [checksum] + [0xEE]
            Serial0.write(0xAA);
            Serial0.write(0xAA);
            for(uint8_t i = 0; i < 4; i++){
              uint8_t temp = (data >> (i*8)) & 0xFF;
              /*
                ToDo:
                try to send the 8 bits data using Serial0
              */
              checksum += temp;
            }
            checksum &= 0xFF;  // keep in 8 bits
            Serial0.write(checksum);
            Serial0.write(0xEE);
          }

          // Debug print over USB
          if (Serial.available()) {
            Serial.print("ls_state");
            Serial.println(data, BIN); // print in binary
            delay(100);
          }

          // Green = system idle
          rgbLEDWrite(0,125,0);
        }
        break;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// ================================
// Helper Functions
// ================================

// Update max/min sensor values during calibration
void scanning(){
  while(!read_done_flag); // wait until new data is ready
  for(uint8_t i = 0; i < LS_count; i++){
    uint16_t reading = result[i];
    /*
    ToDo: assign max_ls[i] and min_ls[i] by finding the maximum and minimum
    */
  }
}

// Save threshold = (max + min) / 2 to EEPROM
void save_avg(){
  memory.clear();
  for(uint8_t i = 0; i < LS_count; i++){
    /*
    ToDo: calculate avg_ls[i] by using the formula average = (max+min)/2
    */
  }
  memory.putBytes("LS_avg", avg_ls, sizeof(avg_ls)); 
}

// Convert sensor readings to 32-bit bitmask
// Bit i = 1 if sensor reading > threshold (bright)
// Bit i = 0 if sensor reading <= threshold (dark)
uint32_t raw_data(){
  uint32_t ls_state = 0;
  while(!read_done_flag);
  for(uint8_t i = 0; i < LS_count; i++){
    /*
    ToDo:
    compare result[i] with avg_ls[i]
    if touch the line, the corrsponding bit in ls_state should be set as 1.
    using bitwise operation
    */
  }
  return ls_state;
}

// ================================
// Main Loop
// ================================
// Nothing here — all work is done in FreeRTOS tasks
void loop() {
  // empty
}
