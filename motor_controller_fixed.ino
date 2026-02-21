#include "Wire.h"
#include "HiTechnicController.h"
#include "HiTechnicDcMotorController.h"
#include "HiTechnicMotor.h"

using DaisyChainPosition = HiTechnicController::DaisyChainPosition;
using MotorPort = HiTechnicDcMotorController::MotorPort;
using RunMode = HiTechnicDcMotorController::RunMode;
using ZeroPowerBehavior = HiTechnicDcMotorController::ZeroPowerBehavior;
using Direction = HiTechnicMotor::Direction;

#define TICKS_PER_REV 1440
#define WHEEL_DIAMETER_IN 4.0
#define DEFAULT_PWR 0.30f

#define TRIG_PIN 12
#define ECHO_PIN 13

#define MAX_ENCODER_JUMP_MULTIPLIER 2.0
#define MAX_VALID_ENCODER_VALUE 100000
#define CONSECUTIVE_BAD_THRESHOLD 3
#define POSITION_TOLERANCE 20

bool ENABLE_BL = true;
bool ENABLE_BR = true;
bool ENABLE_FL = true;
bool ENABLE_FR = true;

HiTechnicDcMotorController mc1(DaisyChainPosition::FIRST);
HiTechnicDcMotorController mc2(DaisyChainPosition::SECOND);

HiTechnicMotor mBL(&mc1, MotorPort::PORT_1);
HiTechnicMotor mBR(&mc2, MotorPort::PORT_2);
HiTechnicMotor mFL(&mc1, MotorPort::PORT_2);
HiTechnicMotor mFR(&mc2, MotorPort::PORT_1);

String serialBuffer = "";
bool movement_commanded = false;
unsigned long movement_start_time = 0;

long target_pos1 = 0;
long target_pos2 = 0;
long target_pos3 = 0;
long target_pos4 = 0;

long last_valid_pos1 = 0;
long last_valid_pos2 = 0;
long last_valid_pos3 = 0;
long last_valid_pos4 = 0;

uint8_t consecutive_bad_count1 = 0;
uint8_t consecutive_bad_count2 = 0;
uint8_t consecutive_bad_count3 = 0;
uint8_t consecutive_bad_count4 = 0;

long inchesToTicks(float inches) {
  float circumference = WHEEL_DIAMETER_IN * 3.14159f;
  float revs = inches / circumference;
  return (long)(revs * TICKS_PER_REV);
}

long degreesToTicks(float degrees) {
  float inches = (degrees / 360.0) * WHEEL_DIAMETER_IN * 3.14159f;
  return inchesToTicks(inches);
}

bool isGarbageValue(long value) {
  if (abs(value) > MAX_VALID_ENCODER_VALUE) return true;
  
  return (value == 50331648 || value == 50528256 || 
          value == 100663296 || value == 101056512 ||
          value == 167772160 || value == 169082880 ||
          value == 67108544 || value == 117440192 ||
          value == 184549056 || value == 218103807);
}

bool isValidEncoderUpdate(long newValue, long lastValue, uint8_t &consecutiveBadCount) {
  if (isGarbageValue(newValue)) {
    consecutiveBadCount++;
    return false;
  }
  
  if (abs(newValue) > MAX_VALID_ENCODER_VALUE) {
    consecutiveBadCount++;
    return false;
  }
  
  if (lastValue != 0) {
    long delta = abs(newValue - lastValue);
    long threshold = abs(lastValue) * MAX_ENCODER_JUMP_MULTIPLIER;
    
    if (delta > threshold && delta > 1000) {
      consecutiveBadCount++;
      
      if (consecutiveBadCount >= CONSECUTIVE_BAD_THRESHOLD) {
        return false;
      }
    }
  }
  
  consecutiveBadCount = 0;
  return true;
}

float getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    return 3.0;
  }
  
  float distance_cm = duration * 0.034 / 2.0;
  float distance_m = distance_cm / 100.0;
  
  if (distance_m < 0.02 || distance_m > 4.0) {
    return 3.0;
  }
  
  return distance_m;
}

void executeMotor(int motorNum, float degrees) {
  long ticks = degreesToTicks(degrees);
  
  movement_commanded = true;
  movement_start_time = millis();
  
  switch(motorNum) {
    case 1:
      if (ENABLE_BL) {
        long current = mBL.getCurrentPosition();
        if (isGarbageValue(current)) {
          Serial.println("Motor 1 (BL): Garbage encoder - skipping command");
          break;
        }
        target_pos1 = current + (-ticks);
        mBL.setTargetPosition(current + (-ticks));
        mBL.setMode(RunMode::RUN_TO_POSITION);
        mBL.setPower(DEFAULT_PWR);
        Serial.print("Motor 1 (BL) target: ");
        Serial.print(target_pos1);
        Serial.println(" ticks");
      }
      break;
    case 2:
      if (ENABLE_BR) {
        long current = mBR.getCurrentPosition();
        if (isGarbageValue(current)) {
          Serial.println("Motor 2 (BR): Garbage encoder - skipping command");
          break;
        }
        target_pos2 = current + ticks;
        mBR.setTargetPosition(current + ticks);
        mBR.setMode(RunMode::RUN_TO_POSITION);
        mBR.setPower(DEFAULT_PWR);
        Serial.print("Motor 2 (BR) target: ");
        Serial.print(target_pos2);
        Serial.println(" ticks");
      }
      break;
    case 3:
      if (ENABLE_FL) {
        long current = mFL.getCurrentPosition();
        if (isGarbageValue(current)) {
          Serial.println("Motor 3 (FL): Garbage encoder - skipping command");
          break;
        }
        target_pos3 = current + ticks;
        mFL.setTargetPosition(current + ticks);
        mFL.setMode(RunMode::RUN_TO_POSITION);
        mFL.setPower(DEFAULT_PWR);
        Serial.print("Motor 3 (FL) target: ");
        Serial.print(target_pos3);
        Serial.println(" ticks");
      }
      break;
    case 4:
      if (ENABLE_FR) {
        long current = mFR.getCurrentPosition();
        if (isGarbageValue(current)) {
          Serial.println("Motor 4 (FR): Garbage encoder - skipping command");
          break;
        }
        target_pos4 = current + (-ticks);
        mFR.setTargetPosition(current + (-ticks));
        mFR.setMode(RunMode::RUN_TO_POSITION);
        mFR.setPower(DEFAULT_PWR);
        Serial.print("Motor 4 (FR) target: ");
        Serial.print(target_pos4);
        Serial.println(" ticks");
      }
      break;
    default:
      Serial.print("Unknown motor: ");
      Serial.println(motorNum);
  }
}

bool isMotorAtTarget(HiTechnicMotor &motor, long target) {
  long current = motor.getCurrentPosition();
  return abs(current - target) <= POSITION_TOLERANCE;
}

void stopAllMotors() {
  if (ENABLE_BL) {
    mBL.setPower(0);
    mBL.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_BR) {
    mBR.setPower(0);
    mBR.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_FL) {
    mFL.setPower(0);
    mFL.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_FR) {
    mFR.setPower(0);
    mFR.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  movement_commanded = false;
}

void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;
  
  Serial.print("Received: ");
  Serial.println(line);
  
  if (line == "STOP") {
    stopAllMotors();
    Serial.println("EMERGENCY STOP - All motors stopped");
    return;
  }
  
  if (line == "RESET") {
    Serial.println("Resetting encoders...");
    stopAllMotors();
    delay(100);
    
    if (ENABLE_BL) mBL.setMode(RunMode::STOP_AND_RESET_ENCODER);
    if (ENABLE_BR) mBR.setMode(RunMode::STOP_AND_RESET_ENCODER);
    if (ENABLE_FL) mFL.setMode(RunMode::STOP_AND_RESET_ENCODER);
    if (ENABLE_FR) mFR.setMode(RunMode::STOP_AND_RESET_ENCODER);
    
    delay(200);
    
    target_pos1 = 0;
    target_pos2 = 0;
    target_pos3 = 0;
    target_pos4 = 0;
    
    last_valid_pos1 = 0;
    last_valid_pos2 = 0;
    last_valid_pos3 = 0;
    last_valid_pos4 = 0;
    
    Serial.println("Encoders reset complete");
    Serial.println("All positions zeroed");
    return;
  }
  
  if (line.charAt(0) == 'B') {
    int idx = 1;
    while (idx < line.length()) {
      while (idx < line.length() && (line[idx] == ' ' || line[idx] == '\t')) idx++;
      if (idx >= line.length()) break;
      
      int motorStart = idx;
      while (idx < line.length() && isDigit(line[idx])) idx++;
      if (motorStart == idx) break;
      int motorNum = line.substring(motorStart, idx).toInt();
      
      while (idx < line.length() && !isDigit(line[idx]) && line[idx] != '-' && line[idx] != '+') idx++;
      if (idx >= line.length()) break;
      
      int degStart = idx;
      if (line[idx] == '-' || line[idx] == '+') idx++;
      while (idx < line.length() && (isDigit(line[idx]) || line[idx] == '.')) idx++;
      
      float degrees = line.substring(degStart, idx).toFloat();
      
      executeMotor(motorNum, degrees);
    }
    Serial.println("Batch command processed.");
  }
  else if (line.charAt(0) == 'M') {
    int motorNum;
    float degrees;
    float speed;
    
    int scanned = sscanf(line.c_str(), "M %d %f %f", &motorNum, &degrees, &speed);
    
    if (scanned >= 2) {
      executeMotor(motorNum, degrees);
      Serial.println("Single motor command processed.");
    }
  }
  else {
    Serial.println("Unknown command format");
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  
  Serial.println("==========================================");
  Serial.println("ROS2 Motor Controller - MECANUM DRIVE");
  Serial.println("Version: Fixed sign inversion + tolerance");
  Serial.println("==========================================");
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("HC-SR04 initialized on pins 12 (TRIG) and 13 (ECHO)");
  
  Wire.begin();
  Wire.setClock(37390L);
  Wire.setWireTimeout(5000, true);
  Serial.println("I2C started at 37390 Hz with 5ms timeout");
  
  Serial.println("Testing motor controllers...");
  mc1.setTimeoutEnabled(false);
  mc2.setTimeoutEnabled(false);
  delay(50);
  Serial.println("Motor controllers configured");
  
  Serial.println("Initializing motors...");
  
  if (ENABLE_BL) {
    mBL.setPower(0);
    mBL.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_BR) {
    mBR.setPower(0);
    mBR.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_FL) {
    mFL.setPower(0);
    mFL.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  if (ENABLE_FR) {
    mFR.setPower(0);
    mFR.setMode(RunMode::RUN_WITHOUT_ENCODER);
  }
  
  delay(100);
  
  if (ENABLE_BL) mBL.setMode(RunMode::STOP_AND_RESET_ENCODER);
  if (ENABLE_BR) mBR.setMode(RunMode::STOP_AND_RESET_ENCODER);
  if (ENABLE_FL) mFL.setMode(RunMode::STOP_AND_RESET_ENCODER);
  if (ENABLE_FR) mFR.setMode(RunMode::STOP_AND_RESET_ENCODER);
  
  delay(200);
  
  Serial.println("Verifying encoder reset...");
  if (ENABLE_BL) {
    Serial.print("BL encoder: ");
    Serial.println(mBL.getCurrentPosition());
  }
  if (ENABLE_BR) {
    Serial.print("BR encoder: ");
    Serial.println(mBR.getCurrentPosition());
  }
  if (ENABLE_FL) {
    Serial.print("FL encoder: ");
    Serial.println(mFL.getCurrentPosition());
  }
  if (ENABLE_FR) {
    Serial.print("FR encoder: ");
    Serial.println(mFR.getCurrentPosition());
  }
  
  if (ENABLE_BL) mBL.setZeroPowerBehavior(ZeroPowerBehavior::BRAKE);
  if (ENABLE_BR) mBR.setZeroPowerBehavior(ZeroPowerBehavior::BRAKE);
  if (ENABLE_FL) mFL.setZeroPowerBehavior(ZeroPowerBehavior::BRAKE);
  if (ENABLE_FR) mFR.setZeroPowerBehavior(ZeroPowerBehavior::BRAKE);
  
  if (ENABLE_BL) mBL.setDirection(Direction::FORWARD);
  if (ENABLE_BR) mBR.setDirection(Direction::FORWARD);
  if (ENABLE_FL) mFL.setDirection(Direction::FORWARD);
  if (ENABLE_FR) mFR.setDirection(Direction::FORWARD);
  
  mc1.setTimeoutEnabled(true);
  mc2.setTimeoutEnabled(true);
  
  Serial.println("==========================================");
  Serial.println("Setup complete. Ready for commands.");
  Serial.println("MOTOR DIRECTIONS: ALL FORWARD (signs handled in software)");
  Serial.print("POSITION TOLERANCE: ±");
  Serial.print(POSITION_TOLERANCE);
  Serial.println(" ticks");
  Serial.println("==========================================");
  
  Serial.println("Testing ultrasonic sensor...");
  for (int i = 0; i < 5; i++) {
    float dist = getUltrasonicDistance();
    Serial.print("Distance reading ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(dist);
    Serial.println(" m");
    delay(200);
  }
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processLine(serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
  
  static unsigned long lastDataReport = 0;
  if (millis() - lastDataReport >= 333) {
    long pos1 = ENABLE_BL ? mBL.getCurrentPosition() : 0;
    long pos2 = ENABLE_BR ? mBR.getCurrentPosition() : 0;
    long pos3 = ENABLE_FL ? mFL.getCurrentPosition() : 0;
    long pos4 = ENABLE_FR ? mFR.getCurrentPosition() : 0;
    
    if (isValidEncoderUpdate(pos1, last_valid_pos1, consecutive_bad_count1)) {
      last_valid_pos1 = pos1;
    }
    if (isValidEncoderUpdate(pos2, last_valid_pos2, consecutive_bad_count2)) {
      last_valid_pos2 = pos2;
    }
    if (isValidEncoderUpdate(pos3, last_valid_pos3, consecutive_bad_count3)) {
      last_valid_pos3 = pos3;
    }
    if (isValidEncoderUpdate(pos4, last_valid_pos4, consecutive_bad_count4)) {
      last_valid_pos4 = pos4;
    }
    
    float distance = getUltrasonicDistance();
    
    Serial.print("DATA ");
    Serial.print(last_valid_pos1); Serial.print(" ");
    Serial.print(last_valid_pos2); Serial.print(" ");
    Serial.print(last_valid_pos3); Serial.print(" ");
    Serial.print(last_valid_pos4); Serial.print(" ");
    Serial.print(distance, 2);
    Serial.println();
    
    lastDataReport = millis();
  }
  
  if (movement_commanded) {
    bool motor1_at_target = true;
    bool motor2_at_target = true;
    bool motor3_at_target = true;
    bool motor4_at_target = true;
    
    if (ENABLE_BL) {
      motor1_at_target = isMotorAtTarget(mBL, target_pos1);
    }
    if (ENABLE_BR) {
      motor2_at_target = isMotorAtTarget(mBR, target_pos2);
    }
    if (ENABLE_FL) {
      motor3_at_target = isMotorAtTarget(mFL, target_pos3);
    }
    if (ENABLE_FR) {
      motor4_at_target = isMotorAtTarget(mFR, target_pos4);
    }
    
    int motors_at_target = 0;
    if (motor1_at_target) motors_at_target++;
    if (motor2_at_target) motors_at_target++;
    if (motor3_at_target) motors_at_target++;
    if (motor4_at_target) motors_at_target++;
    
    static unsigned long lastStatusPrint = 0;
    if ((millis() - lastStatusPrint > 500) && motors_at_target < 3) {
      if (ENABLE_BL) { 
        Serial.print("BL: "); 
        Serial.print(mBL.getCurrentPosition());
        Serial.print("/"); 
        Serial.print(target_pos1);
        Serial.print(motor1_at_target ? " ✓ | " : " | ");
      }
      if (ENABLE_BR) { 
        Serial.print("BR: "); 
        Serial.print(mBR.getCurrentPosition());
        Serial.print("/"); 
        Serial.print(target_pos2);
        Serial.print(motor2_at_target ? " ✓ | " : " | ");
      }
      if (ENABLE_FL) { 
        Serial.print("FL: "); 
        Serial.print(mFL.getCurrentPosition());
        Serial.print("/"); 
        Serial.print(target_pos3);
        Serial.print(motor3_at_target ? " ✓ | " : " | ");
      }
      if (ENABLE_FR) { 
        Serial.print("FR: "); 
        Serial.print(mFR.getCurrentPosition());
        Serial.print("/"); 
        Serial.print(target_pos4);
        Serial.print(motor4_at_target ? " ✓" : "");
      }
      Serial.println();
      lastStatusPrint = millis();
    }
    
    if (motors_at_target >= 3 || (millis() - movement_start_time > 5000)) {
      if (motors_at_target >= 3) {
        Serial.print("Motors at target (");
        Serial.print(motors_at_target);
        Serial.println("/4) - stopping all");
      } else {
        Serial.println("Timeout - stopping all motors");
      }
      if (ENABLE_BL) {
        Serial.print("BL: "); Serial.print(mBL.getCurrentPosition());
        Serial.print("/"); Serial.print(target_pos1);
        Serial.println(motor1_at_target ? " [OK]" : " [--]");
      }
      if (ENABLE_BR) {
        Serial.print("BR: "); Serial.print(mBR.getCurrentPosition());
        Serial.print("/"); Serial.print(target_pos2);
        Serial.println(motor2_at_target ? " [OK]" : " [--]");
      }
      if (ENABLE_FL) {
        Serial.print("FL: "); Serial.print(mFL.getCurrentPosition());
        Serial.print("/"); Serial.print(target_pos3);
        Serial.println(motor3_at_target ? " [OK]" : " [--]");
      }
      if (ENABLE_FR) {
        Serial.print("FR: "); Serial.print(mFR.getCurrentPosition());
        Serial.print("/"); Serial.print(target_pos4);
        Serial.println(motor4_at_target ? " [OK]" : " [--]");
      }
      stopAllMotors();
      Serial.println("Motors stopped");
    }
  }
  
  delay(10);
}
