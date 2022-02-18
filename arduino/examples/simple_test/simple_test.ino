#include <Wire.h>
#include <Servo.h>

#define ESP_AI_CAM_ADDRESS 0x16

#define AI_MODE_CONFIG 1 << 7
#define AI_MODE_CAT_FACE_DETECTION    1
#define AI_MODE_CODE_RECOGNITION      2
#define AI_MODE_COLOR_DETECTION       3
#define AI_MODE_HUMAN_FACE_DETECTION  4
#define AI_MODE_HUMAN_FACE_RECOGITION 5
#define AI_MODE_MOTION_DETECTION      6

#define FRAME_WIDTH  320
#define FRAME_HEIGHT 240

#define SERVO_STEP    5
#define POS_THRESHOLD 1

Servo panServo;
Servo tiltServo;

int panPos = 90;
int tiltPos = 90;
uint8_t noseX, noseY;
uint8_t lastData[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

void print_data(uint8_t *data);
bool isUpdated(uint8_t *newData);

void setup() {
  Serial.begin(115200);
  Serial.println("START!!");
  panServo.attach(6);
  tiltServo.attach(5);
  Wire.begin();
//  Wire.beginTransmission(ESP_AI_CAM_ADDRESS);
//  Wire.write(AI_MODE_CONFIG | AI_MODE_HUMAN_FACE_DETECTION);
//  Wire.endTransmission();
}

void loop() {
  //Read 16 bytes from the slave
  uint8_t len = Wire.requestFrom(ESP_AI_CAM_ADDRESS, 14);
  if (len == 0) {
    delay(100);
    return;
  }
  uint8_t data[len];
  Wire.readBytes(data, len);
//  if (!isUpdated(data)){
//    delay(100);
//    return;
//  }
//  print_data(data);
  noseX = data[8];
  noseY = data[9];

  if (noseX == 255 && noseY == 255){
    delay(100);
    return;
  }
  Serial.print("Detected: (");
  Serial.print(data[8]);
  Serial.print(", ");
  Serial.print(data[9]);
  Serial.println(")");
  if (noseX < (FRAME_WIDTH/2 - POS_THRESHOLD)) {
    Serial.println("On Left");
    panPos += SERVO_STEP;
  } else if (noseX > (FRAME_WIDTH/2 + POS_THRESHOLD)) {
    Serial.println("On Right");
    panPos -= SERVO_STEP;
  }
  if (noseY < (FRAME_HEIGHT/2 - POS_THRESHOLD)) {
    Serial.println("On Bottom");
    tiltPos -= SERVO_STEP;
  } else if (noseY > (FRAME_HEIGHT/2 + POS_THRESHOLD)) {
    Serial.println("On Top");
    tiltPos += SERVO_STEP;
  }
  panServo.write(panPos);
  tiltServo.write(tiltPos);
  delay(10);
}

void print_data(uint8_t * data){
    Serial.print("Detected: (");
    Serial.print(data[0]);
    Serial.print(", ");
    Serial.print(data[0]);
    Serial.print(", ");
    Serial.print(data[2]);
    Serial.print(", ");
    Serial.print(data[3]);
    Serial.println(")");
    Serial.print("    left eye: (");
    Serial.print(data[4]);
    Serial.print(", ");
    Serial.print(data[5]);
    Serial.println(")");
    Serial.print("    right eye: (");
    Serial.print(data[6]);
    Serial.print(", ");
    Serial.print(data[7]);
    Serial.println(")");
    Serial.print("    nose: (");
    Serial.print(data[8]);
    Serial.print(", ");
    Serial.print(data[9]);
    Serial.println(")");
    Serial.print("    mouth left: (");
    Serial.print(data[10]);
    Serial.print(", ");
    Serial.print(data[11]);
    Serial.println(")");
    Serial.print("    mouth right: (");
    Serial.print(data[12]);
    Serial.print(", ");
    Serial.print(data[13]);
    Serial.println(")");
}

bool isUpdated(uint8_t *newData) {
  bool flag = false;
  for (int i=0; i<14; i++){
    if (lastData[i] != newData[i]){
      flag = true;
    }
    lastData[i] = newData[i];
  }
  return flag;
}
