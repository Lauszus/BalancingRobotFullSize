/* Copyright (C) 2015 Kristian Lauszus, TKJ Electronics. All rights reserved.

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

#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS     48
#define TURN_PIXELS    5

#define GREEN          pixels.Color(0, 255, 0)
#define RED            pixels.Color(255, 0, 0)
#define ORANGE         pixels.Color(255, 135, 0)

static const uint8_t turningLeftPin = A2, turningRightPin = A1, pixelsPin = A3;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, pixelsPin, NEO_GRB + NEO_KHZ800);

bool leftState, rightState;

uint32_t timer; // Left and right can't be on at the same time, so we can just use the same timer for both

void setup() {
  pinMode(turningLeftPin, INPUT);
  pinMode(turningRightPin, INPUT);

  pixels.begin();
}

void loop() {
  setMiddleBackOn(); // Set all lights on. This will get overriden by left and right turn signals

  if (digitalRead(turningLeftPin)) {
    if ((int32_t)(millis() - timer) > 250) { // Blink every 250 ms
      timer = millis();
      leftState = !leftState;
    }
    setLeftTurn(leftState);
  }
  else if (digitalRead(turningRightPin)) {
    if ((int32_t)(millis() - timer) > 250) { // Blink every 250 ms
      timer = millis();
      rightState = !rightState;
    }
    setRightTurn(rightState);
  }

  pixels.show();
}

void setMiddleBackOn() {
  for (uint8_t i = 0; i < NUM_PIXELS / 2; i++)
    pixels.setPixelColor(i, RED); // Back LEDs
  for (uint8_t i = NUM_PIXELS / 2; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, GREEN); // Front LEDs
}

void setLeftTurn(bool enable) {
  for (uint8_t i = 0; i < TURN_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
  for (uint8_t i = NUM_PIXELS - TURN_PIXELS; i < NUM_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
}

void setRightTurn(bool enable) {
  for (uint8_t i = NUM_PIXELS / 2 - TURN_PIXELS; i < NUM_PIXELS / 2 + TURN_PIXELS; i++)
    pixels.setPixelColor(i, enable ? ORANGE : 0);
}
