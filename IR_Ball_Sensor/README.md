## **Example 1: `#define` usage - 5 mins**

```cpp
// Example: Using #define to create constants
// Exercise: define LED_PIN as pin 13 and blink at every 100ms
#define DELAY 500 // milliseconds

void setup() {
  //Todo: LED pin as output
  Serial.begin(115200);
  Serial.println("Define Example Started");
}

void loop() {
  //Todo: LED pin turn on(high)
  Serial.println("LED ON");
  delay(DELAY);
  //Todo: LED pin turn off(low)
  Serial.println("LED OFF");
  delay(DELAY);
}
```

**Purpose:** Learn `#define` constants for pins and timing.

---

## **Example 2: Array usage - 10 mins**

```cpp
// Example: Storing multiple sensor values in an array
// Exercise: print digtal sensor state 
#define SENSOR_COUNT 3

int sensorPins[SENSOR_COUNT] = {32, 33, 34};
int sensorValues[SENSOR_COUNT];

void setup() {
  Serial.begin(115200);
  //Reset all sensor values to 0
  for (int i = 0; i < SENSOR_COUNT; i++) {
    sensorValues[i] = 0;
  }
  //Todo: set all pin mode to input mode
  
  Serial.println("Array Example Started");
}

void loop() {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    sensorValues[i] = digitalRead(sensorPins[i]);
    Serial.print("Sensor "); Serial.print(i);
    //Todo:Print all sensorValues by using Serial.print();
  }
  delay(500);
}
```

**Purpose:** Learn how to use arrays to store and access multiple values.

---

## **Example 3: Find Maximum and Minimum - 20 mins**

```cpp
// Example: Find min value in an array
// Exercise: Find max or min value in the array
#define SENSOR_COUNT 3

int values[SENSOR_COUNT] = {5, 9, 3};
bool find_min = true;

void setup() {
  Serial.begin(115200);
  Serial.println("Example Started");
}

int findmin() {
  int minVal = 32767;   // fixed typo (32678 → 32767, safe large number)
  int minIndex = 0;
  for (int i = 0; i < SENSOR_COUNT; i++) {   // fixed: start from 0
    if (values[i] < minVal) {
      minVal = values[i];
      minIndex = i;
    }
  }
  return minIndex;
}

int findmax() {
  // TODO: write a function to find maximum value in array.
  ;
}

void loop() {
  // fixed syntax: Serial.print doesn’t accept multiple params
  Serial.print("Min value: ");
  Serial.print(values[findmin()]);
  Serial.print(", Min index: ");
  Serial.println(findmin());

  // Todo: use if-else and boolean variable find_min
  // if "find_min" == true, find min value, otherwise find max value

  delay(1000);
}

```

**Purpose:** Learn how to scan an array to find the maximum / minimum value and its index using function.

---

## **Example 4: Bitwise Operations - 10 mins**

```cpp
uint8_t strength = 9;  // 0-15
uint8_t index = 2;     // 0-15
uint8_t packet;

void setup() {
  Serial.begin(115200);
  Serial.println("Bitwise Example Started");

  packet = (strength << 4) | (index & 0x0F);
  // TODO: right shift the variable "strength" with 4 bits, 
  // keep the lower 4 bits of the variable "index", 
  // and combine them into 8 bits data called "packet"
  
  Serial.print("Packet byte: 0x"); 
  Serial.println(packet, HEX);
}

void loop() {
  // Nothing here
  ;
}

```

**Purpose:** Learn how to pack two 4-bit numbers into a single byte using bitwise operators.

---

## **Example 5: UART Serial0 send - 15 mins**

```cpp
// Example: Send formatted data via Serial0
// Exercise1: create a MARCO call HEADER with a value of 0xAA
// Exercise2: create two variable called strength and index, each variable only contains 4 bits, combine them into the 8bits variable "data", and send after HEADER.


#define END_BYTE 0xEE

uint8_t data = 0x5A; // example payload
uint8_t checksum;

void setup() {
  Serial0.begin(115200);
  Serial.begin(115200);
  Serial.println("Serial0 Example Started");

  // Calculate checksum (here simple sum of data)
  checksum = data;

  //Todo: send "data" after sending HEADER "two times"
  Serial0.write(checksum);
  Serial0.write(END_BYTE);

  Serial.println("Packet sent via Serial0");
}

void loop() {
  // Nothing here
}
```

**Purpose:** Learn how to send a simple data packet over `Serial0` with header, checksum, and end byte.
