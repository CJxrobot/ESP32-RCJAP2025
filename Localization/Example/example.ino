#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
//提示：因为传感器输出数据频率较快，在下载程序时候，请拔掉传感器或将使能脚拉低（可通过1个IO口控制），停止输出，否则可能引起程序下载超时。
//arduino使用主串口，串口的D0(RX),接模组的TX,模组使能（EN）悬空，如需控制模组的测距速度，arduino的任意IO口，接模组的使能脚（EN）,
//模组EN脚，高电平工作，控制时序见规格书。

*/
char input_str[] = "Range: 103 mm  Amplitude: 15934  Ringdown_flag: 1";
//char input_str[] = "Range: 40 mm  Amplitude: 2000\r\n";
//char input_str[] = "0\r\n";

int target = 0;


// 解析转换之后的变量，range为距离信息，单位是mm，amplitude是幅度信息，ringdown_flag为标志位
int range, amplitude, ringdown_flag;

bool receive_finish=0;
String asc_protocol;

// 字符串解析函数
void parseOutput(const char* output) {
  
    // 检查是否是特定的 "0\r\n" 字符串
    if (strcmp(output, "0\r") == 0) {
        range = amplitude = ringdown_flag = 0;
        return 0;
    } else if (strstr(output, "Range:") != NULL && strstr(output, "Amplitude:") != NULL) {
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
void read_serial(void){
  if (Serial.available() > 0) { // 循环直到没有更多数据可读
    char c = Serial.read(); // 读取一个字符
    if (c!='\n'){
      asc_protocol+=c;
    }
    else
    {
      receive_finish=1;
    }
  }
}
void setup() {
  Serial.begin(115200);

}

void loop() {
  read_serial();
  if (receive_finish){
    const char* asc_protocol_array=asc_protocol.c_str();
    receive_finish=0;
    parseOutput(asc_protocol_array);
    asc_protocol="";
      
    // 用户代码，可以直接引用range, amplitude和ringdown_flag的值
    Serial.print("Range:");
    Serial.print(range);
    Serial.print(",Amplitude:");
    Serial.print(amplitude);
    if (ringdown_flag!=-1){//判断协议是否支持盲区标志
      Serial.print(",Ringdown_flag:");
      Serial.println(ringdown_flag);
    }
    else{
      Serial.println("");
    }
    delay(10);
  }
}

