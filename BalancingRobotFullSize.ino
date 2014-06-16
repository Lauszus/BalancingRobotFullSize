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
#include <Wire.h> // Standard Arduino I2C library
#include <Kalman.h>

#include "BalancingRobotFullSize.h"
#include "Pins.h"
#include "Motor.h"
#include "IMU.h"
#include "EEPROM.h"
#include "Protocol.h"
#include "PID.h"

double pitch; // Angle of the robot
double turningValue; // The turning value of the steering rod

uint32_t kalmanTimer; // Timer used for the Kalman filter
static uint32_t pidTimer; // Timer used for the PID loop

static bool layingDown; // Used to indicate if the robot is laying down or the button is pressed

static double zeroTurning;

double getTurning() {
  double turning = (double)analogRead(A0) / 204.6 - 2.5; // First convert reading to voltage and then subtract 2.5V, as this is the center of the steering wheel
  turning *= cfg.turningScale; // Scale the turning value, so it will actually turn
  //Serial.println(turning);
  return turning;
}

void setup() {
  /* Setup deadman button */
  deadmanButton::SetDirRead();

  /* Setup buzzer pin */
  buzzer::SetDirWrite();

  /* Read the PID values, target angle and other saved values in the EEPROM */
  if (!checkEEPROMVersion())
    readEEPROMValues(); // Only read the EEPROM values if they have not been restored
  else { // Indicate that the EEPROM values have been reset by turning on the buzzer
    buzzer::Set();
    delay(1000);
    buzzer::Clear();
    delay(100); // Wait a little after the pin is cleared
  }

  initSerial();
  initMotors();
  initIMU();

  zeroTurning = getTurning(); // Calibrate turning

  /* Beep to indicate that it is now ready */
  buzzer::Set();
  delay(100);
  buzzer::Clear();

  /* Setup timing */
  kalmanTimer = micros();
  pidTimer = kalmanTimer;
}

void loop () {
  // TODO: Check motor diagnostic pins

  if (dataReady::IsSet()) { // Check is new data is ready
    updateAngle();

    /* Drive motors */
    uint32_t timer = micros();
    // If the robot is laying down, it has to be put in a vertical position before it starts balancing
    // If it's already balancing it has to be ±45 degrees before it stops trying to balance
    // Also make sure that the deadman buttons is pressed
    if (!deadmanButton::IsSet() || (layingDown && (pitch < cfg.targetAngle - 5 || pitch > cfg.targetAngle + 5)) || (!layingDown && (pitch < cfg.targetAngle - 45 || pitch > cfg.targetAngle + 45))) {
      layingDown = true; // The robot is in a unsolvable position, so turn off both motors and wait until it's vertical again
      stopAndReset();
    } else {
      layingDown = false; // It's no longer laying down
      turningValue = getTurning() - zeroTurning;
      updatePID(cfg.targetAngle, 0 /*targetOffset*/, turningValue, (double)(timer - pidTimer) / 1000000.0);
    }
    pidTimer = timer;
  }

  parseSerialData(); // Parse incoming serial data
}
