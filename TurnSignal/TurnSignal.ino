/* Copyright (C) 2015 Kristian Sloth Lauszus. All rights reserved.

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

#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS     48
#define TURN_PIXELS    5

#define GREEN          pixels.Color(0, 255, 0)
#define RED            pixels.Color(255, 0, 0)
#define ORANGE         pixels.Color(255, 135, 0)

static const uint8_t turningLeftPin = A2, turningRightPin = A1, pixelsPin = A3;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, pixelsPin, NEO_GRB + NEO_KHZ800);

bool turnSignalState, turning;
uint32_t timer; // Left and right turn signals can't be on at the same time, so we can just use the same timer for both

void setup() {
  pinMode(turningLeftPin, INPUT);
  pinMode(turningRightPin, INPUT);

  pixels.begin();
}

void loop() {
  setMiddleAndBackOn(); // Set all lights on. This may get overriden by left and right turn signals

  bool turningLeft = digitalRead(turningLeftPin);
  bool turningRight = digitalRead(turningRightPin);
  if (turningLeft || turningRight) {
    turning = true;
    if ((int32_t)(millis() - timer) > 250) { // Blink every 250 ms
      timer = millis();
      turnSignalState = !turnSignalState;
    }
    if (turningLeft)
      setLeftTurn(turnSignalState);
    else if (turningRight)
      setRightTurn(turnSignalState);
  } else if (turning) {
    turning = false;
    uint32_t now = millis();
    delay(250 - min((now > timer ? now - timer : 0), 250)); // Make sure now is larger and the difference is no more than 250
    timer = millis(); // Reset timer
    turnSignalState = false; // Set LED state back to off
  }

  pixels.show();
}

void setMiddleAndBackOn() {
  for (uint8_t i = 0; i < NUM_PIXELS / 2; i++)
    pixels.setPixelColor(i, RED); // Back LEDs
  for (uint8_t i = NUM_PIXELS / 2; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, GREEN); // Front LEDs
}

void setLeftTurn(bool enable) { // The left turn signals are in the ends of the LED strip
  for (uint8_t i = 0; i < TURN_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
  for (uint8_t i = NUM_PIXELS - TURN_PIXELS; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
}

void setRightTurn(bool enable) { // The right turn signals are in the center of the LED strip
  for (uint8_t i = NUM_PIXELS / 2 - TURN_PIXELS; i < NUM_PIXELS / 2 + TURN_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
}
