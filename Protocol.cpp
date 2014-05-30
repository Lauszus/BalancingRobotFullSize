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
  uint8_t length;
  uint8_t cmd;
} msg;
/*
struct pid_t {
  double Kp;
  double Ki;
  double Kd;
} pid;
*/
#define SET_PID 0 // Out message: Kp, Ki, Kd
#define GET_PID 1 // Out message: Kp, Ki, Kd

const char *responseHeader = "$S>"; // Standard response header
//const char *commandHeader = "$S<"; // Standard command header

// Message protocol (Inspired by MultiWii):
// Header: $S>
// Length of data: uint8_t
// cmd: uint8_t
// Data: n uint8_t
// Checksum

// Response:
// Header: $S<
// Length of data: uint8_t
// cmd: uint8_t
// Data: n uint8_t
// Checksum

bool getData(uint8_t *data, uint8_t length);
uint8_t getCheckSum(uint8_t *data, size_t length);

void initSerial() {
  Serial.begin(57600);
  Serial.setTimeout(10); // Only wait 10ms for serial data
}

void parseSerialData() {
  if (Serial.available()) {
    if (Serial.find((char*)responseHeader)) {
      if (Serial.readBytes((char*)&msg, sizeof(msg)) == sizeof(msg)) {
#if 1
        switch (msg.cmd - '0') {
          case GET_PID:
            Serial.print(cfg.Kp);
            Serial.print('\t');
            Serial.print(cfg.Ki);
            Serial.print('\t');
            Serial.println(cfg.Kd);
            break;
          case SET_PID:
            if (msg.length - '0' == 3 /*sizeof(pid)/sizeof(pid.Kp)*/) {
              char input[30];

              size_t pos = Serial.readBytesUntil(';', input, sizeof(input));
              if (pos >= sizeof(input)) {
#if DEBUG
                //Serial.print(pos); Serial.print('\t'); Serial.println(sizeof(input));
                Serial.println(F("Incoming string is too long!"));
#endif
                return;
              }
              input[pos] = '\0'; // Insert null character

              double Kp = atof(strtok(input, ","));
              double Ki = atof(strtok(NULL, ","));
              double Kd = atof(strtok(NULL, ","));
              int checksum = atoi(strtok(NULL, ";"));

              int check = (int)(Kp * 100) ^ (int)(Ki * 100) ^ (int)(Kd * 100);
#if 0
              Serial.print(Kp);
              Serial.print('\t');
              Serial.print(Ki);
              Serial.print('\t');
              Serial.println(Kd);

              Serial.print(F("Checksum: "));
              Serial.print(checksum);
              Serial.print(F(" VS "));
              Serial.println(check);
#endif
              if (check == checksum) {
                cfg.Kp = Kp;
                cfg.Ki = Ki;
                cfg.Kd = Kd;
#if DEBUG
                Serial.print(cfg.Kp);
                Serial.print('\t');
                Serial.print(cfg.Ki);
                Serial.print('\t');
                Serial.println(cfg.Kd);
#endif
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
#else
        switch (msg.cmd) {
          case SET_PID:
            if (msg.length == sizeof(pid) && getData((uint8_t*)&pid, sizeof(pid))) {
              Serial.print(pid.Kp);
              Serial.print('\t');
              Serial.print(pid.Ki);
              Serial.print('\t');
              Serial.println(pid.Kd);
              cfg.Kp = pid.Kp;
              cfg.Ki = pid.Ki;
              cfg.Kd = pid.Kd;
              updateEEPROMValues();
            }
#if DEBUG
            else
              Serial.println(F("SET_PID checksum error"));
#endif
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

bool getData(uint8_t *data, uint8_t length) {
  if (!(Serial.readBytes((char*)data, length) == length))
    return false;
  char checksum;
  if (Serial.readBytes(&checksum, sizeof(checksum)) != sizeof(checksum))
    return false;
  return (getCheckSum((uint8_t*)&msg, sizeof(msg)) ^ getCheckSum(data, length)) == checksum; // The checksum is calculated from the length, command and the data
}

uint8_t getCheckSum(uint8_t *data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++)
    checksum ^= data[i]; // Calculate checksum
  return checksum;
}
