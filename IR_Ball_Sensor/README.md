## **Example 1: `#define` usage**

```cpp
// Example: Using #define to create constants
// Exercise, define LED_PIN as pin 13 and blink at every 100ms
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

## **Example 2: Array usage**

```cpp
// Example: Storing multiple sensor values in an array

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

## **Example 3: Find Maximum**

```cpp
// Example: Find max value in an array

#define SENSOR_COUNT 3

int values[SENSOR_COUNT] = {5, 9, 3};
int maxVal = 0;
int maxIndex = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Find Max Example Started");
}

void loop() {
  for (int i = 1; i < SENSOR_COUNT; i++) {
    if (values[i] > maxVal) {
      maxVal = values[i];
      maxIndex = i;
    }
  }

  Serial.print("Max value: "); Serial.print(maxVal);
  Serial.print(" at index "); Serial.println(maxIndex);

  delay(1000);
}
```

**Purpose:** Learn how to scan an array to find the maximum value and its index.

---

## **Example 4: Bitwise Operations**

```cpp
// Example: Combine two 4-bit numbers into one byte

uint8_t strength = 9;  // 0-15
uint8_t index = 2;     // 0-15
uint8_t packet;

void setup() {
  Serial.begin(115200);
  Serial.println("Bitwise Example Started");

  // Combine upper 4 bits = strength, lower 4 bits = index
  packet = (strength << 4) | (index & 0x0F);

  Serial.print("Packet byte: 0x"); Serial.println(packet, HEX);
}

void loop() {
  // Nothing here
}
```

**Purpose:** Learn how to pack two 4-bit numbers into a single byte using bitwise operators.

---

## **Example 5: UART Serial0 send**

```cpp
// Example: Send formatted data via Serial0

#define HEADER1 0xAA
#define HEADER2 0xAA
#define END_BYTE 0xEE

uint8_t data = 0x5A; // example payload
uint8_t checksum;

void setup() {
  Serial0.begin(115200);
  Serial.begin(115200);
  Serial.println("Serial0 Example Started");

  // Calculate checksum (here simple sum of data)
  checksum = data;

  Serial0.write(HEADER1);
  Serial0.write(HEADER2);
  Serial0.write(data);
  Serial0.write(checksum);
  Serial0.write(END_BYTE);

  Serial.println("Packet sent via Serial0");
}

void loop() {
  // Nothing here
}
```

**Purpose:** Learn how to send a simple data packet over `Serial0` with header, checksum, and end byte.
