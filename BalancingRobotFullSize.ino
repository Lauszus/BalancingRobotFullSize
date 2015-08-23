/* Copyright (C) 2014 Kristian Sloth Lauszus. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Sloth Lauszus
 Web      :  http://www.lauszus.com
 e-mail   :  lauszus@gmail.com
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

#if defined(__AVR_ATmega328P__) && NUM_ANALOG_INPUTS != 8
#error "Please update the Arduino IDE to version 1.5.7 or above - see: https://github.com/arduino/Arduino/pull/2148"
#endif

double turningValue; // The turning value of the steering rod
uint16_t batteryLevel; // Battery level multiplied by 100 i.e. 24.50V becomes 2450

uint32_t kalmanTimer; // Timer used for the Kalman filter
static uint32_t pidTimer; // Timer used for the PID loop

static bool layingDown; // Used to indicate if the robot is laying down or the button is pressed
static double zeroTurning; // Used for calibration of the steer at startup
static uint8_t batteryCounter;

double getTurning() {
  double turning = (double)analogRead(STEER_PIN) / 204.6 - 2.5; // First convert reading to voltage and then subtract 2.5V, as this is the center of the steering wheel
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
#if 0
  if (leftF1::IsSet() || leftF2::IsSet() || rightF1::IsSet() || rightF2::IsSet()) { // Check the diagnostic pins
    buzzer::Set();
    Serial.print(F("Diagnostic error: "));
    Serial.print(leftF1::IsSet());
    Serial.write('\t');
    Serial.print(leftF2::IsSet());
    Serial.write('\t');
    Serial.print(rightF1::IsSet());
    Serial.write('\t');
    Serial.println(rightF2::IsSet());
  }
#endif

  if (dataReady::IsSet()) { // Check is new data is ready
    updateAngle();

    turningValue = getTurning() - zeroTurning; // Update turning value

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
      updatePID(cfg.targetAngle, 0 /*targetOffset*/, turningValue, (double)(timer - pidTimer) / 1000000.0);
    }
    pidTimer = timer;

    if (++batteryCounter > 100) {
      batteryCounter = 0;
      batteryLevel = (double)analogRead(VBAT_PIN) / 204.6 * 780.0; // It is connected to a 68k-10k voltage divider and then we multiply this by 100, so 12.50V will be equal to 1250 - the voltage divider is connected to an op amp configured as a buffer
      if (batteryLevel < 2160 && batteryLevel > 500) // Equal to 3.6V per cell (21.60V in total) - don't turn on if it's below 5V, this means that no battery is connected
        buzzer::Set();
      else
        buzzer::Clear();
      // TODO: Calibrate this value
    }
  } else
    parseSerialData(); // Parse incoming serial data
}
