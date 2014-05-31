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

#include "IMU.h"
#include "BalancingRobotFullSize.h"
#include "Pins.h"
#include "I2C.h"
#include "EEPROM.h"

Kalman kalman;

static const uint8_t IMUAddress = 0x68; // AD0 is logic low on the board

static int16_t accY, accZ, gyroX;
static double gyroXzero;
static uint8_t i2cBuffer[8]; // Buffer for I2C data

static double accAngle, gyroRate, gyroAngle;

bool calibrateGyro();
bool checkMinMax(int16_t *array, uint8_t length, int16_t maxDifference);
void updateIMUValues();

void initIMU() {
  /* Setup IMU */
  initI2c();

  while (i2cRead(IMUAddress, 0x75, i2cBuffer, 1));
  if (i2cBuffer[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    buzzer::Set();
    while (1); // Halt
  }

  while (i2cWrite(IMUAddress, 0x6B, (1 << 7))); // Reset device, this resets all internal registers to their default values
  do {
    while (i2cRead(IMUAddress, 0x6B, i2cBuffer, 1));
  } while (i2cBuffer[0] & (1 << 7)); // Wait for the bit to clear
  delay(5);
  while (i2cWrite(IMUAddress, 0x6B, (1 << 3) | (1 << 0))); // Disable sleep mode, disable temperature sensor and use PLL with X axis gyroscope as clock reference

#if 1
  i2cBuffer[0] = 1; // Set the sample rate to 500Hz - 1kHz/(1+1) = 500Hz
  i2cBuffer[1] = 0x03; // Disable FSYNC and set 44 Hz Acc filtering, 42 Hz Gyro filtering, 1 KHz sampling
#else
  i2cBuffer[0] = 15; // Set the sample rate to 500Hz - 8kHz/(15+1) = 500Hz
  i2cBuffer[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
#endif
  i2cBuffer[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cBuffer[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(IMUAddress, 0x19, i2cBuffer, 4)); // Write to all four registers at once

  /* Enable Data Ready Interrupt on INT pin */
  i2cBuffer[0] = (1 << 5) | (1 << 4); // Enable LATCH_INT_EN and INT_RD_CLEAR
                                      // When this bit is equal to 1, the INT pin is held high until the interrupt is cleared
                                      // When this bit is equal to 1, interrupt status bits are cleared on any read operation
  i2cBuffer[1] = (1 << 0); // Enable DATA_RDY_EN - When set to 1, this bit enables the Data Ready interrupt, which occurs each time a write operation to all of the sensor registers has been completed
  while (i2cWrite(IMUAddress, 0x37, i2cBuffer, 2)); // Write to both registers at once

  dataReady::SetDirRead();

  delay(100); // Wait for the sensor to get ready

  /* Set Kalman and gyro starting angle */
  updateIMUValues();

  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accAngle = (atan2((double)accY - cfg.accYzero, (double)accZ - cfg.accZzero)) * RAD_TO_DEG;

  kalman.setAngle(accAngle); // Set starting angle
  pitch = accAngle;
  gyroAngle = accAngle;

  /* Calibrate gyro zero value */
  while (calibrateGyro()); // Run again if the robot is moved while calibrating
}

bool calibrateGyro() {
  int16_t gyroXbuffer[25];
  for (uint8_t i = 0; i < 25; i++) {
    while (i2cRead(IMUAddress, 0x43, i2cBuffer, 2));
    gyroXbuffer[i] = ((i2cBuffer[0] << 8) | i2cBuffer[1]);
    delay(10);
  }
  if (!checkMinMax(gyroXbuffer, 25, 2000)) {
    Serial.println(F("Gyro calibration error"));
    buzzer::Set();
    return 1;
  }
  for (uint8_t i = 0; i < 25; i++)
    gyroXzero += gyroXbuffer[i];
  gyroXzero /= 25;
  return 0;
}

bool checkMinMax(int16_t *array, uint8_t length, int16_t maxDifference) { // Used to check that the robot is laying still while calibrating
  int16_t min = array[0], max = array[0];
  for (uint8_t i = 1; i < length; i++) {
    if (array[i] < min)
      min = array[i];
    else if (array[i] > max)
      max = array[i];
  }
  return max - min < maxDifference;
}

void updateIMUValues() {
  while (i2cRead(IMUAddress, 0x3D, i2cBuffer, 8));
  accY = ((i2cBuffer[0] << 8) | i2cBuffer[1]);
  accZ = ((i2cBuffer[2] << 8) | i2cBuffer[3]);
  gyroX = ((i2cBuffer[6] << 8) | i2cBuffer[7]);
}

void updateAngle() {
  /* Calculate pitch */
  updateIMUValues();

  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accAngle = (atan2((double)accY - cfg.accYzero, (double)accZ - cfg.accZzero)) * RAD_TO_DEG;

  uint32_t timer = micros();
  // This fixes the -180 to 180 transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((accAngle < -90 && pitch > 90) || (accAngle > 90 && pitch < -90)) {
    kalman.setAngle(accAngle);
    pitch = accAngle;
    gyroAngle = accAngle;
  } else {
    gyroRate = ((double)gyroX - gyroXzero) / 131.0; // Convert to deg/s
    double dt = (double)(timer - kalmanTimer) / 1000000.0;
    gyroAngle += gyroRate * dt; // Gyro angle is only used for debugging
    if (gyroAngle < -180 || gyroAngle > 180)
      gyroAngle = pitch; // Reset the gyro angle when it has drifted too much
    pitch = kalman.getAngle(accAngle, gyroRate, dt); // Calculate the angle using a Kalman filter
  }
  kalmanTimer = timer;
  //Serial.print(accAngle);Serial.print('\t');Serial.print(gyroAngle);Serial.print('\t');Serial.println(pitch);
}
