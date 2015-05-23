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

#include <stdint.h> // Needed for uint8_t, uint16_t etc.

#include "EEPROM.h"
#include "EEPROMAnything.h"
#include "IMU.h"

cfg_t cfg; //  Struct for all the configuration values

//static const char *firmwareVersion = "1.0.0";
static const uint8_t eepromVersion = 2; // EEPROM version - used to restore the EEPROM values if the configuration struct have changed

/* EEPROM Address Definitions */
static const uint8_t versionAddr = 0; // Set the first byte to the EEPROM version
static const uint8_t configAddr = 1; // Save the configuration starting from this location

void restoreEEPROMValues();

bool checkEEPROMVersion() {
  uint8_t version;
  EEPROM_readAnything(versionAddr, version);
  if (version != eepromVersion) { // Check if the EEPROM version matches the current one
    restoreEEPROMValues();
    EEPROM_updateAnything(versionAddr, eepromVersion); // After the default values have been restored, set the flag
    return true; // Indicate that the values have been restored
  }
  return false;
}

void readEEPROMValues() {
  EEPROM_readAnything(configAddr, cfg);

  kalmanPitch.setQangle(cfg.Qangle);
  kalmanPitch.setQbias(cfg.Qbias);
  kalmanPitch.setRmeasure(cfg.Rmeasure);
}

void updateEEPROMValues() {
  EEPROM_updateAnything(configAddr, cfg);

  kalmanPitch.setQangle(cfg.Qangle);
  kalmanPitch.setQbias(cfg.Qbias);
  kalmanPitch.setRmeasure(cfg.Rmeasure);
}

void restoreEEPROMValues() {
  cfg.Kp = 2.8;
  cfg.Ki = 2.0;
  cfg.Kd = 3.5;

  cfg.targetAngle = 0.0;
  cfg.turningScale = 40.0;

  cfg.Qangle = 0.001;
  cfg.Qbias = 0.003;
  cfg.Rmeasure = 0.03;

  cfg.accYzero = cfg.accZzero = 0;
  cfg.leftMotorScaler = cfg.rightMotorScaler = 1;

  updateEEPROMValues();
}
