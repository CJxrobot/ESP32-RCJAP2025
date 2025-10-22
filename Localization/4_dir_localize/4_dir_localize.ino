#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

// --------------------------------------------------------
// Your macros (kept exactly)
// --------------------------------------------------------
#define left_us Serial1
#define left_rx 1
#define left_tx 2 // unused

#define front_us Serial2
#define front_rx 8 
#define front_tx 7 // unused

#define back_us Serial3
#define back_rx 10 
#define back_tx 11 // unused

#define right_us Serial4
#define right_rx 17 
#define right_tx 10 // unused

// --------------------------------------------------------
// Sensor Data Structure
// --------------------------------------------------------
struct SensorData {
  int distance;      // mm
  int amplitude;
  int ringdown;
  bool valid;
};

// 0=front, 1=right, 2=back, 3=left
SensorData sensors[4];
String serialBuffers[4];
bool receiveFlags[4] = {false, false, false, false};

// previous valid robot position (mm)
float prevX = 0, prevY = 0;

// --------------------------------------------------------
// Parsing function
// --------------------------------------------------------
void parseOutput(const char* output, SensorData &data) {
  if (strcmp(output, "0\r") == 0) {
    data.distance = data.amplitude = data.ringdown = 0;
    data.valid = false;
    return;
  }

  const char* rangePtr = strstr(output, "Range:");
  const char* amplitudePtr = strstr(output, "Amplitude:");
  const char* ringdownPtr = strstr(output, "Ringdown_flag:");

  if (rangePtr && amplitudePtr) {
    char* endPtr;
    data.distance = (int)strtol(rangePtr + 6, &endPtr, 10);
    data.amplitude = (int)strtol(amplitudePtr + 10, &endPtr, 10);
    data.ringdown = ringdownPtr ? (int)strtol(ringdownPtr + 14, &endPtr, 10) : -1;

    // Apply offset correction (-5 cm)
    data.distance -= 50;
    if (data.distance < 0) data.distance = 0;

    data.valid = (data.ringdown != 1);
  } else {
    data.distance = data.amplitude = data.ringdown = -1;
    data.valid = false;
  }
}

// --------------------------------------------------------
// Serial data reading helper
// --------------------------------------------------------
void readSerial(HardwareSerial &serial, int id) {
  while (serial.available()) {
    char c = serial.read();
    if (c != '\n') serialBuffers[id] += c;
    else {
      receiveFlags[id] = true;
      break;
    }
  }
}

// --------------------------------------------------------
// Localization logic
// --------------------------------------------------------
void computeLocalization() {
  const float FIELD_W = 1800.0; // mm
  const float FIELD_H = 2400.0; // mm

  float front = sensors[0].valid ? sensors[0].distance : NAN;
  float right = sensors[1].valid ? sensors[1].distance : NAN;
  float back  = sensors[2].valid ? sensors[2].distance : NAN;
  float left  = sensors[3].valid ? sensors[3].distance : NAN;

  float x = prevX;
  float y = prevY;

  // --- X calculation (Left & Right) ---
  if (sensors[1].valid && sensors[3].valid) {
    // both valid -> average
    x = ((FIELD_W/2 - right) + (-FIELD_W/2 + left)) / 2.0;
  } 
  else if (sensors[1].valid) {
    // only right valid
    x = FIELD_W/2 - right;
  } 
  else if (sensors[3].valid) {
    // only left valid
    x = -FIELD_W/2 + left;
  }

  // --- Y calculation (Front & Back) ---
  if (sensors[0].valid && sensors[2].valid) {
    // both valid -> average
    y = ((FIELD_H/2 - front) + (-FIELD_H/2 + back)) / 2.0;
  } 
  else if (sensors[0].valid) {
    y = FIELD_H/2 - front;
  } 
  else if (sensors[2].valid) {
    y = -FIELD_H/2 + back;
  }

  // if valid, update stored position
  if (!isnan(x)) prevX = x;
  if (!isnan(y)) prevY = y;

  // --- Output in cm ---
  Serial.print("Position => X: ");
  Serial.print(prevX / 10.0, 1);
  Serial.print(" cm, Y: ");
  Serial.print(prevY / 10.0, 1);
  Serial.println(" cm");
}

// --------------------------------------------------------
// Setup
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);

  left_us.begin(115200, SERIAL_8N1, left_rx, left_tx);
  front_us.begin(115200, SERIAL_8N1, front_rx, front_tx);
  back_us.begin(115200, SERIAL_8N1, back_rx, back_tx);
  right_us.begin(115200, SERIAL_8N1, right_rx, right_tx);

  Serial.println("Ultrasonic localization initialized");
}

// --------------------------------------------------------
// Main loop
// --------------------------------------------------------
void loop() {
  // Read from all sensors
  readSerial(front_us, 0);
  readSerial(right_us, 1);
  readSerial(back_us, 2);
  readSerial(left_us, 3);

  // Parse new data
  for (int i = 0; i < 4; i++) {
    if (receiveFlags[i]) {
      parseOutput(serialBuffers[i].c_str(), sensors[i]);
      serialBuffers[i] = "";
      receiveFlags[i] = false;
    }
  }

  // Compute localization
  computeLocalization();
  delay(50);
}
