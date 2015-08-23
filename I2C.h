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

#ifndef _i2c_h_
#define _i2c_h_

void initI2c();
uint8_t i2cWrite(uint8_t address, uint8_t registerAddress, uint8_t data, bool sendStop = true);
uint8_t i2cWrite(uint8_t address, uint8_t registerAddress, uint8_t *data, uint8_t length, bool sendStop = true);
uint8_t i2cRead(uint8_t address, uint8_t registerAddress, uint8_t *data, uint8_t nbytes);

#endif
