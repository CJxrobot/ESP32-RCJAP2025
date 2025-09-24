// ============================
// Example 0 Template: LED + Serial + Serial0 UART
// ============================

#define LED_BUILTIN 2

void setup() {
  // Initialize LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize Serial for debugging
  Serial.begin(/* TODO: baud rate */);

  // Initialize Serial0 for UART sending
  Serial0.begin(/* TODO: baud rate */);

  Serial.println("Setup complete");
  Serial0.println("Setup complete");
}

void loop() {
  // ============================
  // 1. Toggle LED
  // ============================
  digitalWrite(LED_BUILTIN, HIGH);
  delay(/* TODO: LED ON time in ms */);
  digitalWrite(LED_BUILTIN, LOW);
  delay(/* TODO: LED OFF time in ms */);

  // ============================
  // 2. Print debug message to Serial
  // ============================
  Serial.println(/* TODO: message to print */);

  // ============================
  // 3. Send message over Serial0 (UART)
  // ============================
  Serial0.print(/* TODO: start marker, e.g., '$' */);
  Serial0.print(/* TODO: message content */);
  Serial0.println(/* TODO: end marker, e.g., '#' */);

  // ============================
  // 4. Optional delay before next loop
  // ============================
  delay(/* TODO: loop delay in ms */);
}
