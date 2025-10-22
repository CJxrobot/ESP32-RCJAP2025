// 1 8 10 17 -> left front back right -> ultrasonic sensor 
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



void parseOutput(const char* output) {
  
    // 检查是否是特定的 "0\r\n" 字符串
    if (strcmp(output, "0\r") == 0) {
        range = amplitude = ringdown_flag = 0;
        return 0;
    } 
    else if (strstr(output, "Range:") != NULL && strstr(output, "Amplitude:") != NULL) {
        // 找到输出字符串中 "Range:" 和 "Amplitude:" 的位置
        const char* rangePtr = strstr(output, "Range:");
        const char* amplitudePtr = strstr(output, "Amplitude:");
        const char* ringdownPtr = strstr(output, "Ringdown_flag:");

        if (ringdownPtr == NULL) {
            ringdown_flag = -1; // 如果没有 "Ringdown_flag:"，则默认值为 -1
        } else {
            // 找到ringdown_flag的值的起始位置
            const char* ringdownStartPtr = ringdownPtr + strlen("Ringdown_flag:");
            // 从字符串中解析出ringdown_flag的值
            char* ringdownEndPtr;
            ringdown_flag = (int)strtol(ringdownStartPtr, &ringdownEndPtr, 10);
        }

        // 找到range和amplitude的值的起始位置
        const char* rangeStartPtr = rangePtr + strlen("Range:");
        const char* amplitudeStartPtr = amplitudePtr + strlen("Amplitude:");

        // 从字符串中解析出range的值
        char* rangeEndPtr;
        range = (int)strtol(rangeStartPtr, &rangeEndPtr,10);

        // 从字符串中解析出amplitude的值
        char* amplitudeEndPtr;
        amplitude = (int)strtol(amplitudeStartPtr, &amplitudeEndPtr, 10);
    } else {
        // 如果字符串格式不正确，将range、amplitude和ringdown_flag都设置为-1
        range = amplitude = ringdown_flag = -1;
    }
}

void setup(){
  Serial.begin(115200);
  left_us.begin(115200, SERIAL_8N1, left_rx, left_tx);
  fonrt_us.begin(115200, SERIAL_8N1, front_rx, front_tx);
  back_us.begin(115200, SERIAL_8N1, back_rx, back_tx);
  right_us.begin(115200, SERIAL_8N1, right_rx, right_tx);
}
