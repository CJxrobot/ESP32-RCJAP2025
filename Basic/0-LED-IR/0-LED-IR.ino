// Example 0: LED + IR Reading + Serial Print

#define LED_BUILTIN 2
#define IR_COUNT 2
int ir_pins[IR_COUNT] = {32, 33};  // two IR sensors
int ir_values[IR_COUNT];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);  // Debug messages
}

void loop() {
  // Toggle LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  Serial.println("testing");
}
