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
#include "EEPROM.h"

struct msg_t {
  uint8_t cmd;
  uint8_t length;
} msg;

struct pid_t {
  uint16_t Kp;
  uint16_t Ki;
  uint16_t Kd;
} pid;

#define SET_PID 0 // In message: Kp, Ki, Kd
#define GET_PID 1 // Out message: Kp, Ki, Kd

const char *commandHeader = "$S>"; // Standard command header
const char *responseHeader = "$S<"; // Standard response header

bool getData(uint8_t *data, uint8_t length);
void sendData(uint8_t *data, uint8_t length);
uint8_t getCheckSum(uint8_t *data, size_t length);

void initSerial() {
  Serial.begin(57600);
  Serial.setTimeout(10); // Only wait 10ms for serial data
}

void parseSerialData() {
  if (Serial.available()) {
    if (Serial.find((char*)commandHeader)) {
      if (Serial.readBytes((uint8_t*)&msg, sizeof(msg)) == sizeof(msg)) {
        switch (msg.cmd) {
          case GET_PID:
            if (msg.length == 0) {
              if (getData(NULL, 0)) { // This will read the data and check the checksum
                msg.cmd = GET_PID;
                msg.length = sizeof(pid);
                pid.Kp = cfg.Kp * 100;
                pid.Ki = cfg.Ki * 100;
                pid.Kd = cfg.Kd * 100;
                sendData((uint8_t*)&pid, sizeof(pid));
              }
#if DEBUG
              else
                Serial.println(F("GET_PID checksum error"));
#endif
            }
            break;
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

// All floats/doubles are multiplied by 100 before sending

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
  Serial.write(msg.cmd);
  Serial.write(msg.length);
  Serial.write(data, length);
  Serial.write(getCheckSum((uint8_t*)&msg, sizeof(msg)) ^ getCheckSum(data, length));
  Serial.println(); // Print new line as well
}

uint8_t getCheckSum(uint8_t *data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++)
    checksum ^= data[i]; // Calculate checksum
  return checksum;
}
