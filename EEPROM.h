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

#ifndef _eeprom_h_
#define _eeprom_h_

#include <stdint.h> // Needed for uint8_t, uint16_t etc.

// This struct will store all the configuration values
typedef struct {
  double Kp, Ki, Kd; // PID variables
  double targetAngle; // Resting angle of the robot
  uint8_t turningScale; // Set the turning scale value
  double Qangle, Qbias, Rmeasure; // Kalman filter values
  double accYzero, accZzero; // Accelerometer zero values
  double leftMotorScaler, rightMotorScaler; // Used if there is difference between the motors
} cfg_t;

extern cfg_t cfg;

bool checkEEPROMVersion();
void readEEPROMValues();
void updateEEPROMValues();

#endif
