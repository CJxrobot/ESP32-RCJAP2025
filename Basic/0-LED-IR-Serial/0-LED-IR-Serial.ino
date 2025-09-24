// Example 0: LED + IR Reading + Serial Print

#define LED_BUILTIN 2
#define IR_COUNT 2
int ir_pins[IR_COUNT] = {32, 33};  // two IR sensors
int ir_values[IR_COUNT];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < IR_COUNT; i++) {
    pinMode(ir_pins[i], INPUT_PULLUP);  // IR sensors
  }

  Serial.begin(115200);  // Debug messages
}

void loop() {
  // Toggle LED
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  // Read IR values
  for (int i = 0; i < IR_COUNT; i++) {
    ir_values[i] = !digitalRead(ir_pins[i]);  // active LOW
  }

  // Print IR values
  Serial.print("IR: ");
  for (int i = 0; i < IR_COUNT; i++) {
    Serial.print(ir_values[i]);
    Serial.print(" ");
  }
  Serial.println();

  delay(500);
}
