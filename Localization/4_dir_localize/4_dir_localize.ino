#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// -----------------------------
// UART Pin Assignments
// -----------------------------
#define left_us   Serial1
#define left_rx   1
#define left_tx   2  // unused

#define front_us  Serial2
#define front_rx  8
#define front_tx  7  // unused

#define back_us   Serial3
#define back_rx   10
#define back_tx   11 // unused

#define right_us  Serial4
#define right_rx  17
#define right_tx  16 // unused

// -----------------------------
// Each sensor’s data variables
// -----------------------------
int left_range = -1, left_amplitude = -1, left_ringdown = -1;
int front_range = -1, front_amplitude = -1, front_ringdown = -1;
int back_range = -1, back_amplitude = -1, back_ringdown = -1;
int right_range = -1, right_amplitude = -1, right_ringdown = -1;

// Buffers and flags for UART data
String buffer_left, buffer_front, buffer_back, buffer_right;
bool finish_left = false, finish_front = false, finish_back = false, finish_right = false;

// -----------------------------
// String parsing function
// -----------------------------
void parseOutput(const char* output, int &range, int &amplitude, int &ringdown_flag) {
    if (strcmp(output, "0\r") == 0) {
        range = amplitude = ringdown_flag = 0;
        return;
    } 
    else if (strstr(output, "Range:") != NULL && strstr(output, "Amplitude:") != NULL) {
        const char* rangePtr = strstr(output, "Range:");
        const char* amplitudePtr = strstr(output, "Amplitude:");
        const char* ringdownPtr = strstr(output, "Ringdown_flag:");

        if (ringdownPtr == NULL) ringdown_flag = -1;
        else {
            const char* ringdownStartPtr = ringdownPtr + strlen("Ringdown_flag:");
            char* ringdownEndPtr;
            ringdown_flag = (int)strtol(ringdownStartPtr, &ringdownEndPtr, 10);
        }

        const char* rangeStartPtr = rangePtr + strlen("Range:");
        const char* amplitudeStartPtr = amplitudePtr + strlen("Amplitude:");

        char* rangeEndPtr;
        range = (int)strtol(rangeStartPtr, &rangeEndPtr, 10);

        char* amplitudeEndPtr;
        amplitude = (int)strtol(amplitudeStartPtr, &amplitudeEndPtr, 10);
    } 
    else {
        range = amplitude = ringdown_flag = -1;
    }
}

// -----------------------------
// Generic non-blocking serial reader
// -----------------------------
void readSerial(HardwareSerial &serial, String &buffer, bool &finished) {
    while (serial.available() > 0) {
        char c = serial.read();
        if (c != '\n') buffer += c;
        else finished = true;
    }
}

// -----------------------------
// Setup
// -----------------------------
void setup() {
    Serial.begin(115200);

    left_us.begin(115200, SERIAL_8N1, left_rx, left_tx);
    front_us.begin(115200, SERIAL_8N1, front_rx, front_tx);
    back_us.begin(115200, SERIAL_8N1, back_rx, back_tx);
    right_us.begin(115200, SERIAL_8N1, right_rx, right_tx);

    Serial.println("Ultrasonic sensors initialized.");
}

// -----------------------------
// Loop
// -----------------------------
void loop() {
    // Read data from all sensors
    readSerial(left_us, buffer_left, finish_left);
    readSerial(front_us, buffer_front, finish_front);
    readSerial(back_us, buffer_back, finish_back);
    readSerial(right_us, buffer_right, finish_right);

    // --- LEFT ---
    if (finish_left) {
        parseOutput(buffer_left.c_str(), left_range, left_amplitude, left_ringdown);
        Serial.printf("[LEFT] Range:%d  Amp:%d  Ring:%d\n", left_range, left_amplitude, left_ringdown);
        buffer_left = "";
        finish_left = false;
    }

    // --- FRONT ---
    if (finish_front) {
        parseOutput(buffer_front.c_str(), front_range, front_amplitude, front_ringdown);
        Serial.printf("[FRONT] Range:%d  Amp:%d  Ring:%d\n", front_range, front_amplitude, front_ringdown);
        buffer_front = "";
        finish_front = false;
    }

    // --- BACK ---
    if (finish_back) {
        parseOutput(buffer_back.c_str(), back_range, back_amplitude, back_ringdown);
        Serial.printf("[BACK] Range:%d  Amp:%d  Ring:%d\n", back_range, back_amplitude, back_ringdown);
        buffer_back = "";
        finish_back = false;
    }

    // --- RIGHT ---
    if (finish_right) {
        parseOutput(buffer_right.c_str(), right_range, right_amplitude, right_ringdown);
        Serial.printf("[RIGHT] Range:%d  Amp:%d  Ring:%d\n", right_range, right_amplitude, right_ringdown);
        buffer_right = "";
        finish_right = false;
    }

    delay(1); // small yield to keep loop stable
}
