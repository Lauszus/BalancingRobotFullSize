/* Copyright (C) 2014 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
*/

#include <Arduino.h>

#include "Protocol.h"
#include "BalancingRobotFullSize.h"
#include "IMU.h"
#include "EEPROM.h"
#include "PID.h"
#include "Pins.h"

struct msg_t {
  uint8_t cmd;
  uint8_t length;
} __attribute__((packed)) msg;

struct pid_t {
  uint16_t Kp;
  uint16_t Ki;
  uint16_t Kd;
} __attribute__((packed)) pid;

struct target_t {
  int16_t targetAngle; // Note that this can be negative as well
} __attribute__((packed)) target;

struct turning_t {
  uint8_t turningScale;
} __attribute__((packed)) turning;

struct kalman_t {
  uint16_t Qangle;
  uint16_t Qbias;
  uint16_t Rmeasure;
} __attribute__((packed)) kalman;

struct info_t {
  uint16_t speed;
  int16_t current; // Note that this can be negative as well
  int16_t turning; // Note that this can be negative as well
  uint16_t battery;
  uint32_t runTime;
} __attribute__((packed)) info;

struct imu_t {
  uint16_t acc;
  uint16_t gyro;
  uint16_t kalman;
} __attribute__((packed)) imu;

#define SET_PID     0
#define GET_PID     1
#define SET_TARGET  2
#define GET_TARGET  3
#define SET_TURNING 4
#define GET_TURNING 5
#define SET_KALMAN  6
#define GET_KALMAN  7

#define START_INFO  8
#define STOP_INFO   9
#define START_IMU  10
#define STOP_IMU   11

const char *commandHeader = "$S>"; // Standard command header
const char *responseHeader = "$S<"; // Standard response header

static bool sendSpeed, sendImu;
static uint32_t speedTimer, imuTimer;

bool getData(uint8_t *data, uint8_t length);
void sendData(uint8_t *data, uint8_t length);
uint8_t getCheckSum(uint8_t *data, size_t length);

void initSerial() {
  Serial.begin(57600);
  Serial.setTimeout(10); // Only wait 10ms for serial data
}

// TODO: Remove all debugging
void parseSerialData() {
  if (Serial.available()) {
    if (Serial.find((char*)commandHeader)) {
      if (Serial.readBytes((uint8_t*)&msg, sizeof(msg)) == sizeof(msg)) {
        switch (msg.cmd) {
          case SET_PID:
            if (msg.length == sizeof(pid)) { // Make sure that it has the right length
              if (getData((uint8_t*)&pid, sizeof(pid))) { // This will read the data and check the checksum
                cfg.Kp = (double)pid.Kp / 100.0;
                cfg.Ki = (double)pid.Ki / 100.0;
                cfg.Kd = (double)pid.Kd / 100.0;
                updateEEPROMValues();
              }
#if DEBUG
              else
                Serial.println(F("SET_PID checksum error"));
#endif
            }
#if DEBUG
            else {
              Serial.print(F("SET_PID length error: "));
              Serial.println(msg.length);
            }
#endif
            break;

          case GET_PID:
            if (msg.length == 0) {
              if (getData(NULL, 0)) { // This will read the data and check the checksum
                msg.cmd = GET_PID;
                msg.length = sizeof(pid);
                pid.Kp = cfg.Kp * 100.0;
                pid.Ki = cfg.Ki * 100.0;
                pid.Kd = cfg.Kd * 100.0;
                sendData((uint8_t*)&pid, sizeof(pid));
              }
#if DEBUG
              else
                Serial.println(F("GET_PID checksum error"));
#endif
            }
            break;

          case SET_TARGET:
            if (msg.length == sizeof(target)) { // Make sure that it has the right length
              if (getData((uint8_t*)&target, sizeof(target))) { // This will read the data and check the checksum
                cfg.targetAngle = (double)target.targetAngle / 100.0;
                updateEEPROMValues();
              }
#if DEBUG
              else
                Serial.println(F("SET_TARGET checksum error"));
#endif
            }
#if DEBUG
            else {
              Serial.print(F("SET_TARGET length error: "));
              Serial.println(msg.length);
            }
#endif
            break;

          case GET_TARGET:
            if (msg.length == 0) {
              if (getData(NULL, 0)) { // This will read the data and check the checksum
                msg.cmd = GET_TARGET;
                msg.length = sizeof(target);
                target.targetAngle = cfg.targetAngle * 100.0;
                sendData((uint8_t*)&target, sizeof(target));
              }
#if DEBUG
              else
                Serial.println(F("GET_PID checksum error"));
#endif
            }
            break;

          case SET_TURNING:
            if (msg.length == sizeof(turning)) { // Make sure that it has the right length
              if (getData((uint8_t*)&turning, sizeof(turning))) { // This will read the data and check the checksum
                cfg.turningScale = turning.turningScale;
                updateEEPROMValues();
              }
#if DEBUG
              else
                Serial.println(F("SET_TURNING checksum error"));
#endif
            }
#if DEBUG
            else {
              Serial.print(F("SET_TURNING length error: "));
              Serial.println(msg.length);
            }
#endif
            break;

          case GET_TURNING:
            if (msg.length == 0) {
              if (getData(NULL, 0)) { // This will read the data and check the checksum
                msg.cmd = GET_TURNING;
                msg.length = sizeof(turning);
                turning.turningScale = cfg.turningScale;
                sendData((uint8_t*)&turning, sizeof(turning));
              }
#if DEBUG
              else
                Serial.println(F("GET_TURNING checksum error"));
#endif
            }
            break;

          case SET_KALMAN:
            if (msg.length == sizeof(kalman)) { // Make sure that it has the right length
              if (getData((uint8_t*)&kalman, sizeof(kalman))) { // This will read the data and check the checksum
                cfg.Qangle = kalman.Qangle / 10000.0;
                cfg.Qbias = kalman.Qbias / 10000.0;
                cfg.Rmeasure = kalman.Rmeasure / 10000.0;
                updateEEPROMValues();
              }
#if DEBUG
              else
                Serial.println(F("SET_KALMAN checksum error"));
#endif
            }
#if DEBUG
            else {
              Serial.print(F("SET_KALMAN length error: "));
              Serial.println(msg.length);
            }
#endif
            break;

          case GET_KALMAN:
            if (msg.length == 0) {
              if (getData(NULL, 0)) { // This will read the data and check the checksum
                msg.cmd = GET_KALMAN;
                msg.length = sizeof(kalman);
                kalman.Qangle = cfg.Qangle * 10000.0;
                kalman.Qbias = cfg.Qbias * 10000.0;
                kalman.Rmeasure = cfg.Rmeasure * 10000.0;
                sendData((uint8_t*)&kalman, sizeof(kalman));
              }
#if DEBUG
              else
                Serial.println(F("GET_KALMAN checksum error"));
#endif
            }
            break;

          case START_INFO:
            if (msg.length == 0) {
              if (getData(NULL, 0)) // This will read the data and check the checksum
                sendSpeed = true;
#if DEBUG
              else
                Serial.println(F("START_INFO checksum error"));
#endif
            }
            break;

          case STOP_INFO:
            if (msg.length == 0) {
              if (getData(NULL, 0)) // This will read the data and check the checksum
                sendSpeed = false;
#if DEBUG
              else
                Serial.println(F("STOP_INFO checksum error"));
#endif
            }
            break;

          case START_IMU:
            if (msg.length == 0) {
              if (getData(NULL, 0)) // This will read the data and check the checksum
                sendImu = true;
#if DEBUG
              else
                Serial.println(F("START_IMU checksum error"));
#endif
            }
            break;

          case STOP_IMU:
            if (msg.length == 0) {
              if (getData(NULL, 0)) // This will read the data and check the checksum
                sendImu = false;
#if DEBUG
              else
                Serial.println(F("STOP_IMU checksum error"));
#endif
            }
            break;

#if DEBUG
          default:
            Serial.print(F("Unknown command: "));
            Serial.println(msg.cmd);
            break;
#endif
        }
      }
    }
  }

  if (sendSpeed && millis() - speedTimer > 100) {
    speedTimer = millis();
    msg.cmd = START_INFO;
    msg.length = sizeof(info);
    info.speed = constrain(abs(PIDValue), 0, 100.0) * 100.0;
    if (deadmanButton::IsSet()) {
      double CS = ((double)analogRead(CS1_PIN) / 204.6 - 2.5) / 0.066 * 100.0; // 66mV/A and then multiply by 100.0
      CS += ((double)analogRead(CS2_PIN) / 204.6 - 2.5) / 0.066 * 100.0;
      info.current = CS;
    } else
      info.current = 0; // When the reset button is held low on the motor drivers, the current sensor will give out an incorrect value
    info.turning = turningValue * 100.0;
    info.battery = batteryLevel;
    info.runTime = speedTimer;
    sendData((uint8_t*)&info, sizeof(info));
  } else if (sendImu && millis() - imuTimer > 100) {
    imuTimer = millis();
    msg.cmd = START_IMU;
    msg.length = sizeof(imu);
    imu.acc = accAngle * 100.0;
    imu.gyro = gyroAngle * 100.0;
    imu.kalman = pitch * 100.0;
    sendData((uint8_t*)&imu, sizeof(imu));
  }
}

// Message protocol (Inspired by MultiWii):
// Request:
// Header: $S>
// cmd: uint8_t
// n length of data: uint8_t
// Data: n uint8_t
// Checksum (calculated from cmd, length and data)

// Response:
// Header: $S<
// cmd: uint8_t
// n length of data: uint8_t
// Data: n uint8_t
// Checksum (calculated from cmd, length and data)
// Carriage return and line feed ("\r\n")

// All floats/doubles are multiplied by 100 before sending
// Except the Kalman values which are multiplied by 10000 before sending

bool getData(uint8_t *data, uint8_t length) {
  if (Serial.readBytes(data, length) != length) // Read data into buffer
    return false;
  uint8_t checksum;
  if (Serial.readBytes(&checksum, sizeof(checksum)) != sizeof(checksum)) // Read the checksum
    return false;
  return (getCheckSum((uint8_t*)&msg, sizeof(msg)) ^ getCheckSum(data, length)) == checksum; // The checksum is calculated from the length, command and the data
}

void sendData(uint8_t *data, uint8_t length) {
  Serial.write(responseHeader);
  Serial.write((uint8_t*)&msg, sizeof(msg));
  Serial.write(data, length);
  Serial.write(getCheckSum((uint8_t*)&msg, sizeof(msg)) ^ getCheckSum(data, length)); // The checksum is calculated from the length, command and the data
  Serial.println(); // Print carriage return and line feed as well, so it is easy to figure out the line ending in Java
}

uint8_t getCheckSum(uint8_t *data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++)
    checksum ^= data[i]; // Calculate checksum
  return checksum;
}
