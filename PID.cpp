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

#include "PID.h"
#include "BalancingRobotFullSize.h"
#include "Motor.h"
#include "EEPROM.h"

double lastError; // Store last angle error
double integratedError; // Store integrated error

void updatePID(double restAngle, double offset, double turning, double dt) {
  /* Update PID values */
  double error = (restAngle - pitch);
  double pTerm = cfg.Kp * error;
  integratedError += error * dt;
  double iTerm = (cfg.Ki * 100.0) * constrain(integratedError, -1.0, 1.0); // Limit the integrated error
  double dTerm = (cfg.Kd / 100.0) * (error - lastError) / dt;
  lastError = error;
  double PIDValue = pTerm + iTerm + dTerm;

#if 0 // TODO: Estimate velocity and steer
  /* Steer robot sideways */
  if (turning < 0) { // Left
    turning += abs((double)wheelVelocity / velocityScaleTurning); // Scale down at high speed
    if (turning > 0)
      turning = 0;
  }
  else if (turning > 0) { // Right
    turning -= abs((double)wheelVelocity / velocityScaleTurning); // Scale down at high speed
    if (turning < 0)
      turning = 0;
  }
#endif
  
  double PIDLeft = PIDValue + turning;
  double PIDRight = PIDValue - turning;

  PIDLeft *= cfg.leftMotorScaler; // Compensate for difference in some of the motors
  PIDRight *= cfg.rightMotorScaler;
  
  //Serial.print(PIDLeft); Serial.write('\t'); Serial.println(PIDRight); 

  /* Set PWM Values */
  if (PIDLeft >= 0)
    moveMotor(left, forward, PIDLeft);
  else
    moveMotor(left, backward, -PIDLeft);
  if (PIDRight >= 0)
    moveMotor(right, forward, PIDRight);
  else
    moveMotor(right, backward, -PIDRight);
}
