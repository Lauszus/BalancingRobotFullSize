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

#ifndef _pins_h_
#define _pins_h_

// I use the pins defined in avrpins.h from the USB Host library which I am also one of the developers of
// This allows to read and write directly to the port registers instead of using Arduino's slow digitalRead()/digitalWrite() functions
// The original source is available here: https://github.com/felis/USB_Host_Shield_2.0/blob/master/avrpins.h
// I do this to save processing power - see this page for more information: http://www.billporter.info/ready-set-oscillate-the-fastest-way-to-change-arduino-pins/
// Also see the Arduino port manipulation guide: http://www.arduino.cc/en/Reference/PortManipulation
#include "avrpins.h"

/* Pin connect to INT on the MPU-6050 */
#define dataReady P2

#define buzzer P7 // Buzzer used for audio feedback
#define deadmanButton P16 // (A2) Deadman button

/* Left motor */
#define leftDir P12
#define leftPWM P10
#define leftPWMReg OCR1B

/* Right motor */
#define rightDir P8
#define rightPWM P9
#define rightPWMReg OCR1A

/* Pins connected to the motor drivers diagnostic pins */
/*#define leftF1 P5
#define leftF2 P6*/

#define rightF1 P3
#define rightF2 P4

/* Analog pins */
#define STEER_PIN A0
#define VBAT_PIN  A1
#define CS1_PIN   A6
//#define CS2_PIN   A7

/* Turn signals */
#define turningLeft P5
#define turningRight P6

#endif
