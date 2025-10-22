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





void setup(){
  Serial.begin(115200);
  left_us.begin(115200, SERIAL_8N1, left_rx, left_tx);
  fonrt_us.begin(115200, SERIAL_8N1, front_rx, front_tx);
  back_us.begin(115200, SERIAL_8N1, back_rx, back_tx);
  right_us.begin(115200, SERIAL_8N1, right_rx, right_tx);
}
