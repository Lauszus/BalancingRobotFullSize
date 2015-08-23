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

final int minIMUAngle = 0, maxIMUAngle = 360;

void drawGraph() {
  background(255); // Set background to white

  stroke(200); // Grey
  for (int i = 0; i < gyro.length; i++) { // Draw graph paper
    line(mainWidth + i * 10, 0, mainWidth + i * 10, height);
    line(mainWidth, i * 10, width, i * 10);
  }

  stroke(0); // Black
  for (int i = 1; i <= 3; i++)
    line(mainWidth, height / 4 * i, width, height / 4 * i); // Draw lines indicating 90 deg, 180 deg, and 270 deg

  if (stringGyro != null)
    gyro[gyro.length - 1] = map(float(trim(stringGyro)), minIMUAngle, maxIMUAngle, 0, height); // Convert to an float and map to the screen height, then save in buffer
  if (stringAcc != null)
    acc[acc.length - 1] = map(float(trim(stringAcc)), minIMUAngle, maxIMUAngle, 0, height); // Convert to an float and map to the screen height, then save in buffer
  if (stringKalman != null)
    kalman[kalman.length - 1] = map(float(trim(stringKalman)), minIMUAngle, maxIMUAngle, 0, height); // Convert to an float and map to the screen height, then save in buffer

  noFill();

  // Draw acceleromter x-axis
  stroke(255, 0, 0); // Red
  beginShape();
  for (int i = 0; i < acc.length; i++)
    vertex(i * graphWidth / (acc.length - 1) + mainWidth, height - acc[i]);
  endShape();

  // Draw gyro x-axis
  stroke(0, 255, 0); // Green
  beginShape();
  for (int i = 0; i < gyro.length; i++)
    vertex(i * graphWidth / (gyro.length - 1) + mainWidth, height - gyro[i]);
  endShape();

  // Draw kalman filter x-axis
  stroke(0, 0, 255); // Blue
  beginShape();
  for (int i = 0; i < kalman.length; i++)
    vertex(i * graphWidth / (kalman.length - 1) + mainWidth, height - kalman[i]);
  endShape();

  for (int i = 1; i < acc.length; i++) { // Put all data one array back
    acc[i - 1] = acc[i];
    gyro[i - 1] = gyro[i];
    kalman[i - 1] = kalman[i];
  }
}
