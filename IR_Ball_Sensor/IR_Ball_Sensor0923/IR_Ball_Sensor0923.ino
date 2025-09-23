#define PI 3.14159265
#define IR_Pin_Count 10
uint8_t ir_pins[IR_Pin_Count] = {1,3,5,7,8,10,11,13,15,17};
uint16_t ir_degree[IR_Pin_Count] = {300,340,30,70,85,95,110,150,200,240};
float cos_deg[IR_Pin_Count], sin_deg[IR_Pin_Count];
float IR_offset = 90;
#define DEBUG
#define OFFSET

void setup() {
	Serial1.begin(115200);
	Serial.begin(115200);
  delay(1000);
  for(uint8_t i = 0; i < IR_Pin_Count; i++){
    pinMode(ir_pins[i], INPUT);
    float temp = ir_degree[i] * PI / 180;
    cos_deg[i] = cos(temp);
    sin_deg[i] = sin(temp);
    Serial.printf("%d , %f, %f\n", ir_degree[i], cos_deg[i], sin_deg[i]);
  }
	Serial.println("init done");
  delay(100);
}

void loop() {
  int ir_weight[IR_Pin_Count] = {0}; // Initialize weight array
	float ball_degree = -1; 
	float sum = 0;
	uint16_t strength = 0;

	for(uint8_t c = 0; c < 5; c++){
    unsigned long int timer = micros();
    do{
      sum = 0;
      for(uint8_t i = 0; i < IR_Pin_Count; i++){
        if(digitalRead(ir_pins[i]) == LOW){ // Check if IR sensor is triggered
          ir_weight[i]++;
        } 
        sum += ir_weight[i];
			}
		} while(micros() - timer <= 833);
	}

  float x = 0, y = 0;
  if(sum != 0){
    for(uint8_t i = 0; i < IR_Pin_Count; i++){
      x += ir_weight[i] * cos_deg[i];
      y += ir_weight[i] * sin_deg[i];
    }
    x = x / sum;
    y = y / sum;
    ball_degree = fmod((atan2(y, x) * 180 / PI) + 360, 360);
  }
  else{
    ball_degree = 999; // No detection
  }

#ifdef OFFSET
    // Apply offset only if ball_degree is valid
	if(ball_degree >= 0 && ball_degree < 140 && ball_degree > 60){
		ball_degree = ball_degree - (IR_offset - 90); // Corrected offset logic
	}
#endif
	if((int)(ball_degree) == 999){
		Serial1.printf("N\n");
	}
	else{
		Serial1.printf("B%d$%d#\n", (int)(ball_degree), (int)(sum * 0.1));
	}
#ifdef DEBUG
  Serial.println("\n--------------");
  Serial.printf("&%d$%d#\n", (int)(ball_degree), (int)(sum * 0.1));
	for (int i = 0; i < IR_Pin_Count; i++){
		Serial.printf("%d = %4d", i, ir_weight[i]);
		Serial.println();
	}
	Serial.printf("Maxball_degree = %f , %f\n", ball_degree, sum);
	Serial.println("--------------");
	Serial.flush();
#endif
}
