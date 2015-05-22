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

#include "Motor.h"
#include "Pins.h"

static const uint16_t PWM_FREQUENCY = 20000; // Set the PWM frequency to 20kHz
static const uint16_t PWMVALUE = F_CPU / PWM_FREQUENCY / 2; // The frequency is given by F_CPU/(2*N*ICR) - where N is the prescaler, prescaling is used so the frequency is given by F_CPU/(2*ICR) - ICR = F_CPU/PWM_FREQUENCY/2

void setPWM(Command motor, uint16_t dutyCycle);

void initMotors() {
  /* Set the motor driver diagnostic pins to inputs */
  /*leftF1::SetDirRead();
  leftF2::SetDirRead();*/
  rightF1::SetDirRead();
  rightF2::SetDirRead();

  /* Setup motor pins to output */
  leftPWM::SetDirWrite();
  leftDir::SetDirWrite();
  rightPWM::SetDirWrite();
  rightDir::SetDirWrite();

  /* Set PWM frequency to 20kHz - see the datasheet http://www.atmel.com/Images/Atmel-8271-8-bit-AVR-Microcontroller-ATmega48A-48PA-88A-88PA-168A-168PA-328-328P_datasheet_Complete.pdf page 128-138 */
  // Set up PWM, Phase and Frequency Correct on pin 9 (OC1A) & pin 10 (OC1B) with ICR1 as TOP using Timer1
  TCCR1B = (1 << WGM13) | (1 << CS10); // Set PWM Phase and Frequency Correct with ICR1 as TOP and no prescaling
  ICR1 = PWMVALUE; // ICR1 is the TOP value - this is set so the frequency is equal to 20kHz

  /* Enable PWM on pin 9 (OC1A) & pin 10 (OC1B) */
  // Clear OC1A/OC1B on compare match when up-counting
  // Set OC1A/OC1B on compare match when down-counting
  TCCR1A = (1 << COM1A1) | (1 << COM1B1);

  stopMotor(left);
  stopMotor(right);
}

void moveMotor(Command motor, Command direction, double speedRaw) { // Speed is a value in percentage 0-100%
  if (speedRaw > 100)
    speedRaw = 100.0;
  setPWM(motor, speedRaw * ((double)PWMVALUE) / 100.0); // Scale from 0-100 to 0-PWMVALUE
  if (motor == left) {
    if (direction == forward)
      leftDir::Clear();
    else
      leftDir::Set();
  } else {
    if (direction == forward)
      rightDir::Set();
    else
      rightDir::Clear();
  }
}

void stopMotor(Command motor) {
  setPWM(motor, 0); // Set low
}

void setPWM(Command motor, uint16_t dutyCycle) { // dutyCycle is a value between 0-ICR1
  if (motor == left)
    leftPWMReg = dutyCycle;
  else
    rightPWMReg = dutyCycle;
}
