#include <Wire.h>
#include <Servo.h>
#include <SoftPWM.h>

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

#define SERVO_STEP    3
#define POS_THRESHOLD 20

#define MOTOR_1_A 8
#define MOTOR_1_B 9
#define MOTOR_2_A 10
#define MOTOR_2_B 11
#define MOTOR_LEFT_REVERSE 0
#define MOTOR_RIGHT_REVERSE 0

Servo panServo;
Servo tiltServo;

int panPos = 90;
int tiltPos = 60;
uint8_t noseX, noseY;
uint8_t lastData[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

void print_data(uint8_t *data);
void motorSetup();
void setMotorPower(int8_t left, int8_t right);

void setup() {
  Serial.begin(9600);
  Serial.println("START!!");
  panServo.attach(6);
  tiltServo.attach(5);
  motorSetup();
  Wire.begin();
  panServo.write(panPos);
  tiltServo.write(tiltPos);
//  Wire.beginTransmission(ESP_AI_CAM_ADDRESS);
//  Wire.write(AI_MODE_CONFIG | AI_MODE_HUMAN_FACE_DETECTION);
//  Wire.endTransmission();
}

int motorSpeed = 0;

void loop() {
  uint8_t len = Wire.requestFrom(ESP_AI_CAM_ADDRESS, 14);
  if (len == 0) {
    delay(100);
    return;
  }
  uint8_t data[len];
  Wire.readBytes(data, len);
//  print_data(data);

  uint8_t width = data[2] - data[0];
  noseX = data[8];
  noseY = data[9];

  if (noseX == 255 && noseY == 255){
    setMotorPower(0, 0);
    delay(100);
    return;
  }
  Serial.print("Detected: (");
  Serial.print(noseX);
  Serial.print(", ");
  Serial.print(noseX);
  Serial.print(")   Width: ");
  Serial.println(width);
  if (noseX < (FRAME_WIDTH/2 - POS_THRESHOLD)) {
    Serial.println("On Right");
    panPos += SERVO_STEP;
    setMotorPower(0, motorSpeed);
  } else if (noseX > (FRAME_WIDTH/2 + POS_THRESHOLD)) {
    Serial.println("On Left");
    panPos -= SERVO_STEP;
    setMotorPower(motorSpeed, 0);
  } else {
    setMotorPower(0, 0);
  }
  if (noseY < (FRAME_HEIGHT/2 - POS_THRESHOLD)) {
    Serial.println("On Top");
    tiltPos -= SERVO_STEP;
  } else if (noseY > (FRAME_HEIGHT/2 + POS_THRESHOLD)) {
    Serial.println("On Bottom");
    tiltPos += SERVO_STEP;
  }
  panServo.write(panPos);
  tiltServo.write(tiltPos);
  delay(120);
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

void motorSetup(){
  SoftPWMBegin();
  SoftPWMSet(MOTOR_1_A, 0);
  SoftPWMSet(MOTOR_1_B, 0);
  SoftPWMSet(MOTOR_2_A, 0);
  SoftPWMSet(MOTOR_2_B, 0);
  SoftPWMSetFadeTime(ALL, 1000, 1000);
}

void setMotorPower(int8_t left, int8_t right){
  left = map(left, -100, 100, -255, 255);
  right = map(right, -100, 100, -255, 255);
  if (MOTOR_LEFT_REVERSE) left = -left;
  if (MOTOR_RIGHT_REVERSE) right = -right;
  if (left > 0) {
    SoftPWMSet(MOTOR_2_A, abs(left));
    SoftPWMSet(MOTOR_2_B, 0);
  } else {
    SoftPWMSet(MOTOR_2_A, 0);
    SoftPWMSet(MOTOR_2_B, abs(left));
  }
  if (right > 0) {
    SoftPWMSet(MOTOR_1_A, 0);
    SoftPWMSet(MOTOR_1_B, abs(right));
  } else {
    SoftPWMSet(MOTOR_1_A, abs(right));
    SoftPWMSet(MOTOR_1_B, 0);
  }
}
