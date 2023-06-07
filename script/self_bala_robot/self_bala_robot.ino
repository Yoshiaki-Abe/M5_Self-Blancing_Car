#define M5STACK_MPU9250
#include <M5Stack.h>
#include <BlynkSimpleEsp32_BLE.h>
#include "Stepper.h"
#include "Kalman.h"
#include "PIDController.h"

#define DT 0.002//制御周期

#define MICRO_STEP 32 //1 or 2 or 4 or 8 or 16 or 32
#define BAT_PIN 35
#define EN_PIN 26

#define ANGLE_OFFSET 0.7

//PID gain
#define ANGLE_KP 0.8
#define ANGLE_KI 0.0
#define ANGLE_KD 0.045

#define SPEED_KP 1.5
#define SPEED_KI 0.0
#define SPEED_KD 0.0

//PID
PIDController angle_pid(DT, 50.0, -50.0);
PIDController speed_pid(DT, 10.0, -10.0);
PIDController pos_pid(DT, 10.0, -10.0);
int control_mode = 1; // 1: position 2: speed

//Stepper
portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE blynk_read_mux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE blynk_write_mux = portMUX_INITIALIZER_UNLOCKED;
Stepper stepperL(16, 17, MICRO_STEP, &timer_mux);
Stepper stepperR(2, 5, MICRO_STEP, &timer_mux, true);
void IRAM_ATTR StepTimerFunc1() { stepperR.TimerStep(); }
void IRAM_ATTR StepTimerFunc2() { stepperL.TimerStep(); }
float motorL_spd = 0.0;
float motorR_spd = 0.0;
float kp = 0.0, ki = 0.0, kd = 0.0;
float avg_step = 0.0;

//Blynk
char blynk_auth[] = "o0xwmMGzwjUX4V1L7uW_QJs_KXWdlyHO";
int joystick_x = 512, joystick_y = 512;
int dt = 0;
uint8_t power = 0;

//IMU
Kalman kalman;
float gyr_offset = 0.0;
float angle = 0.0;
//Battery
float bat_voltage = 15.0;

BLYNK_WRITE(V1)
{
  float x, y;
  x = param[0].asInt();
  y = param[1].asInt();
  joystick_x = x;
  joystick_y = y;
}
BLYNK_WRITE(V7)
{
  float k;
  k = param.asInt();
  power = k;
}
/*
BLYNK_WRITE(V4)
{
  float k;
  k = param.asFloat();
  kp = k;
}

BLYNK_WRITE(V5)
{
  float k;
  k = param.asFloat();
  ki = k;
}

BLYNK_WRITE(V6)
{
  float k;
  k = param.asFloat();
  kd = k;
}
*/
//IMU初期化
void InitIMU()
{
  Wire.begin();
  Wire.setClock(400000);
  M5.IMU.Init();
  for(int i = 0; i < 1000; i++) {
    float gyr_x, gyr_y, gyr_z;
    M5.IMU.getGyroData(&gyr_x, &gyr_y, &gyr_z);
    gyr_offset -= gyr_x;
    delay(1);
  }
  gyr_offset /= 1000.0;
  kalman.setAngle(0.0);
}

//IMUから角度推定
void ReadIMU()
{
  float acc_x, acc_y, acc_z, acc_angle;
  float gyr_x, gyr_y, gyr_z;
  static float pre_gyr_x = 0.0;
  M5.IMU.getAccelData(&acc_x, &acc_y, &acc_z);
  M5.IMU.getGyroData(&gyr_x, &gyr_y, &gyr_z);
  acc_y -= 0.01; acc_z -= 0.08;
  gyr_x += gyr_offset;
  gyr_x = 0.5 * gyr_x + 0.5 * pre_gyr_x;
  acc_angle = atan2(-acc_z, acc_y) * 180.0 / PI;
  angle = kalman.getAngle(acc_angle, gyr_x, DT);
  pre_gyr_x = gyr_x;
}

void ReadBatteryVoltage()
{
  int adc = analogRead(BAT_PIN);
  bat_voltage = 0.05 * (adc / 103.325) + 0.95 * bat_voltage;
}

//Blynkデータ書き込み
void BlynkWrite()
{
  float _angle, _bat_voltage;
  int _dt;
  _dt  = dt;
  _angle = angle;
  _bat_voltage = bat_voltage;
  //Blynk.virtualWrite<float>(V2, _angle);
  Blynk.virtualWrite<float>(V3, _bat_voltage);
  //Blynk.virtualWrite<int>(V8, _dt);
}

//マルチタスク用
void BlynkTask(void *args)
{
  while(1) {
    Blynk.run();
    BlynkWrite();
    delay(200);
  }
}

void setup()
{
  M5.begin();
  //GPIO
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(BAT_PIN, INPUT);
  dacWrite(25, 0);  //消音
  //Stepper
  stepperR.Init(0, StepTimerFunc1);
  stepperL.Init(1, StepTimerFunc2);
  //Blynk
  Blynk.setDeviceName("M5Stack_Grey");
  Blynk.begin(blynk_auth);
  xTaskCreatePinnedToCore(BlynkTask, "BlynkTask", 8192, NULL, 10, NULL, 0);
  //IMU
  InitIMU();
  //PID
  angle_pid.SetParam(ANGLE_KP, ANGLE_KI, ANGLE_KD);
  speed_pid.SetParam(SPEED_KP*1e-2, SPEED_KI*1e-2, SPEED_KD*1e-2);
}

void loop()
{
  static bool power_toggle = false;
  unsigned long timer_us = micros();

  ReadIMU();
  ReadBatteryVoltage();

  //水平に近いときはモータ出力OFF
  if(abs(angle) > 45 | power == 0) {
    digitalWrite(EN_PIN, HIGH);
    motorL_spd = 0.0;
    motorR_spd = 0.0;
    stepperL.SetSpeed(0.0);
    stepperR.SetSpeed(0.0);
    power_toggle = false;
    while(micros() - timer_us <= DT * 1e6) delayMicroseconds(1);
    return;
  } else if(!power_toggle) {
    power_toggle = true;
    digitalWrite(EN_PIN, LOW);
  }
  
  //ステアリング
  static float steer = 0.0;
  steer = 0.4 * (joystick_x - 512.0) / 512.0 * 150.0 + 0.6 * steer;
  //前後移動
  static float target_speed = 0.0;
  if(abs(joystick_y) < 5) joystick_y = 0.0;
  target_speed = 0.7 * target_speed + 0.3 * (joystick_y - 512.0) * 600.0 / 512.0;
  float target_angle = 0.0;
  if(abs(target_speed) < 1) {
    target_angle = 0.0;
  } else {
    target_angle = speed_pid.Output(target_speed, (motorL_spd + motorR_spd) / 2.0);
  }
  //モータ出力計算
  float motor_acc = -angle_pid.Output(target_angle + ANGLE_OFFSET, angle);
  motorL_spd += motor_acc;
  motorR_spd += motor_acc;
  //出力制限
  if(motorL_spd > 800) motorL_spd = 800.0;
  else if(motorL_spd < -800) motorL_spd = -800.0;
  if(motorR_spd > 800) motorR_spd = 800.0;
  else if(motorR_spd < -800) motorR_spd = -800.0;
  //速度出力
  stepperL.SetSpeed(motorL_spd + steer);
  stepperR.SetSpeed(motorR_spd - steer);
  //制御周期調整
  //dt = micros() - timer_us;
  while(micros() - timer_us <= DT * 1e6) delayMicroseconds(1);
}
