// ============================
// Example 0 Template: LED + Serial + Serial0 UART (using write)
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
  Serial0.println("Setup complete");  // Can also use write
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
  // 3. Send message over Serial0 using write
  // ============================
  Serial0.write(/* TODO: start marker, e.g., 0xAA */);
  Serial0.write(/* TODO: message content byte, e.g., 0x01 */);
  Serial0.write(/* TODO: message content byte, e.g., 0x02 */);
  Serial0.write(/* TODO: end marker, e.g., 0xEE */);

  // ============================
  // 4. Optional delay before next loop
  // ============================
  delay(/* TODO: loop delay in ms */);
}
