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

#ifndef _motor_h_
#define _motor_h_

#include <stdint.h> // Needed for uint8_t, uint16_t etc.

enum Command {
  forward,
  backward,
  left,
  right,
};

/* Left motor */
#define leftDir P8
#define leftPWM P9

/* Right motor */
#define rightDir P12
#define rightPWM P10

/* Pins connected to the motor drivers diagnostic pins */
/*#define leftF1 P5
#define leftF2 P6

#define rightF1 P5
#define rightF2 P6*/

void initMotors();
void moveMotor(Command motor, Command direction, double speedRaw);
void stopMotor(Command motor);
void stopAndReset();

#endif
