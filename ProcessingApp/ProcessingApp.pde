/* Copyright (C) 2012-2014 Kristian Lauszus, TKJ Electronics. All rights reserved.

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

import processing.serial.*;
import controlP5.*;
import java.awt.event.*;
import java.awt.Image;

ControlP5 controlP5;

PFont font10, font25, font30;
Textfield P, I, D, targetAngle, maxAngle, turnScale, Qangle, Qbias, Rmeasure;
CheckBox checkBox;

String firmwareVer = "", eepromVer = "", mcu = "", voltage = "", minutes = "", seconds = "";

final boolean useDropDownLists = true; // Set if you want to use the dropdownlist or not
byte defaultComPort = 0; // You should change this as well if you do not want to use the dropdownlist

// Dropdown list
DropdownList dropdownList; // Define the variable ports as a Dropdownlist.
Serial serial; // Define the variable port as a Serial object.
int portNumber = -1; // The dropdown list will return a float value, which we will connvert into an int. We will use this int for that.

boolean connectedSerial;

String stringGyro, stringAcc, stringKalman;

// We will store 101 readings
float[] acc = new float[101];
float[] gyro = new float[101];
float[] kalman = new float[101];

boolean drawValues; // This is set to true whenever there is any new data

final int mainWidth = 337; // Width of the main control panel
final int graphWidth = 700; // Width of the graph

String portnameFile = "SerialPort.txt"; // Name of file for last connected serial port

void setup() {
  registerMethod("dispose", this); // Called automatically before shutting down

  frame.setTitle("Balanduino Processing App");
  frame.setIconImage((Image) loadImage("data/logo.png").getNative());

  controlP5 = new ControlP5(this);
  size(mainWidth + graphWidth, 510);

  font10 = loadFont("EuphemiaUCAS-Bold-10.vlw");
  font25 = loadFont("EuphemiaUCAS-Bold-25.vlw");
  font30 = loadFont("EuphemiaUCAS-Bold-30.vlw");

  /* For setting the PID values etc. */
  P = controlP5.addTextfield("P")
               .setPosition(10, 165)
               .setSize(35, 20)
               .setFocus(true)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  I = controlP5.addTextfield("I")
               .setPosition(50, 165)
               .setSize(35, 20)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  D = controlP5.addTextfield("D")
               .setPosition(90, 165)
               .setSize(35, 20)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  targetAngle = controlP5.addTextfield("targetAngle")
                         .setPosition(130, 165)
                         .setSize(35, 20)
                         //.setInputFilter(ControlP5.FLOAT) // This can be negative as well
                         .setAutoClear(false)
                         .clear();

  /* Settings */
  maxAngle = controlP5.addTextfield("maxAngle")
               .setPosition(10, 255)
               .setSize(40, 20)
               .setInputFilter(ControlP5.INTEGER)
               .setAutoClear(false)
               .clear();

  turnScale = controlP5.addTextfield("turnScale")
               .setPosition(55, 255)
               .setSize(35, 20)
               .setInputFilter(ControlP5.INTEGER)
               .setAutoClear(false)
               .clear();

  /* Kalman filte values */
  Qangle = controlP5.addTextfield("Qangle")
               .setPosition(10, 340)
               .setSize(40, 20)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  Qbias = controlP5.addTextfield("Qbias")
               .setPosition(55, 340)
               .setSize(40, 20)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  Rmeasure = controlP5.addTextfield("Rmeasure")
               .setPosition(100, 340)
               .setSize(40, 20)
               .setInputFilter(ControlP5.FLOAT)
               .setAutoClear(false)
               .clear();

  /* Buttons */
  controlP5.addButton("abort")
           .setPosition(10, 480)
           .setSize(40, 20);

  controlP5.addButton("continueAbort") // We have to call it something else, as continue is protected
           .setPosition(55, 480)
           .setSize(50, 20)
           .setCaptionLabel("continue");

  controlP5.addButton("submit")
           .setPosition(146, 455)
           .setSize(94, 20)
           .setCaptionLabel("Submit values");

  controlP5.addButton("restoreDefaults")
           .setPosition(146, 480)
           .setSize(94, 20)
           .setCaptionLabel("Restore defaults");

  for (int i = 0; i < acc.length; i++) { // center all variables
    acc[i] = height / 2;
    gyro[i] = height / 2;
    kalman[i] = height / 2;
  }

  //println(Serial.list()); // Used for debugging
  if (useDropDownLists)
    initDropdownlist();
  else { // If useDropDownLists is false, it will connect automatically at startup
    portNumber = defaultComPort;
    connect();
  }

  drawGraph(); // Draw graph at startup
}

void draw() {
  /* Draw Graph */
  if (connectedSerial && drawValues) {
    drawValues = false;
    drawGraph();
  }

  /* Remote contol */
  fill(0);
  stroke(0);
  rect(0, 0, mainWidth, height);
  fill(0, 102, 153);
  textSize(25);
  textFont(font25);
  textAlign(CENTER);
  text("Press buttons to steer", mainWidth / 2, 55);

  /* Set PID value etc. */
  fill(0, 102, 153);
  textSize(30);
  textFont(font30);
  textAlign(CENTER);
  text("Set PID Values:", mainWidth / 2, 155);
  text("Settings:", mainWidth / 2, 240);
  text("Kalman values:", mainWidth / 2, 330);

  fill(255, 255, 255);
  textSize(10);
  textFont(font10);
  textAlign(LEFT);
  text("Firmware: " + firmwareVer + " EEPROM: " + eepromVer + " MCU: " + mcu, 10, 410);
  String runtime;
  if (!minutes.isEmpty() && !seconds.isEmpty())
    runtime =  minutes + " min " + seconds + " sec";
  else
    runtime = "";
  text("Battery level: " + voltage + " Runtime: " + runtime, 10, 430);
}

void abort() {
  if (connectedSerial) {
    serial.write("A;");
    println("Abort");
  } else
    println("Establish a serial connection first!");
}

void continueAbort() {
  if (connectedSerial) {
    serial.write("C");
    println("Continue");
  } else
    println("Establish a serial connection first!");
}

void submit() {
  setAllValues();  // Set all values
  getAllValues(); // Get all values back again
}

void restoreDefaults() {
  if (connectedSerial) {
    serial.write("CR;"); // Restore values
    println("RestoreDefaults");
    delay(10);
  } else
    println("Establish a serial connection first!");
}

byte[] bytes;
boolean append;

void serialEvent(Serial serial) {
  if (append)
    bytes = concat(bytes, serial.readBytes());
  else
    bytes = serial.readBytes();
  int[] data = new int[bytes.length];
  for (int i = 0; i < data.length; i++)
    data[i] = bytes[i] & 0xFF; // Cast to unsigned value
  //println(data);

  if (new String(bytes).startsWith(responseHeader)) {
    int cmd = data[responseHeader.length()];
    int length = data[responseHeader.length() + 1];
    if (length != (data.length -  responseHeader.length() - 5)) {
      append = true; // If it's not the correct length, then the rest will come in the next package
      return;
    }
    int input[] = new int[length];
    int i;
    for (i = 0; i < length; i++)
      input[i] = data[i + responseHeader.length() + 2];
    int checksum = data[i + responseHeader.length() + 2];

    if (checksum == (cmd ^ length ^ getChecksum(input))) {
      switch(cmd) {
        case GET_PID:
          int Kp = input[0] | (input[1] << 8);
          int Ki = input[2] | (input[3] << 8);
          int Kd = input[4] | (input[5] << 8);

          P.setText(Float.toString((float)Kp / 100.0));
          I.setText(Float.toString((float)Ki / 100.0));
          D.setText(Float.toString((float)Kd / 100.0));

          println(Kp + " " + Ki + " " + Kd);
          break;
        case GET_TARGET:
          int target = input[0] | ((byte)input[1] << 8); // This can be negative as well

          targetAngle.setText(Float.toString((float)target / 100.0));

          println(target);
          break;
        case GET_TURNING:
          turnScale.setText(Integer.toString(input[0]));

          println(input[0]);
          break;
        case GET_KALMAN:
          int QangleValue = input[0] | (input[1] << 8);
          int QbiasValue = input[2] | (input[3] << 8);
          int RmeasureValue = input[4] | (input[5] << 8);

          Qangle.setText(Float.toString((float)QangleValue / 10000.0));
          Qbias.setText(Float.toString((float)QbiasValue / 10000.0));
          Rmeasure.setText(Float.toString((float)RmeasureValue / 10000.0));

          println(QangleValue + " " + QbiasValue + " " + RmeasureValue);
          break;
        default:
          println("Unknown command");
          break;
      }
    } else
      println("Checksum error!");
  } else {
    println("Wrong header!");
    println(new String(bytes));
  }
    

/*
  else if (input[0].equals("I") && input.length == 4) { // Info
    firmwareVer = input[1];
    eepromVer = input[2];
    mcu  = input[3];
  } else if (input[0].equals("R") && input.length == 3) { // Status response
    voltage  = input[1] + 'V';
    String runtime = input[2];
    minutes = str((int)floor(float(runtime)));
    seconds = str((int)(float(runtime) % 1 / (1.0 / 60.0)));
  } else if (input[0].equals("V") && input.length == 4) { // IMU data
    stringAcc = input[1];
    stringGyro = input[2];
    stringKalman = input[3];
  }
*/
  serial.clear();  // Empty the buffer
  drawValues = true; // Draw the graph
  append = false;
}

void keyPressed() {
  if (key == TAB) { //'\t'
    if (P.isFocus()) {
      P.setFocus(false);
      I.setFocus(true);
    } else if (I.isFocus()) {
      I.setFocus(false);
      D.setFocus(true);
    } else if (D.isFocus()) {
      D.setFocus(false);
      targetAngle.setFocus(true);
    } else if (targetAngle.isFocus()) {
      targetAngle.setFocus(false);
      P.setFocus(true);
    }
    else if (maxAngle.isFocus()) {
      maxAngle.setFocus(false);
      turnScale.setFocus(true);
    } else if (turnScale.isFocus()) {
      turnScale.setFocus(false);
      maxAngle.setFocus(true);
    }
    else if (Qangle.isFocus()) {
      Qangle.setFocus(false);
      Qbias.setFocus(true);
    } else if (Qbias.isFocus()) {
      Qbias.setFocus(false);
      Rmeasure.setFocus(true);
    } else if (Rmeasure.isFocus()) {
      Rmeasure.setFocus(false);
      Qangle.setFocus(true);
    }
    else
      P.setFocus(true);
  }
  else if (key == ENTER) { // '\n'
    if (connectedSerial)
      submit(); // If we are connected, send the values
    else
      connect(); // If not try to connect
  }
  else if (key == ESC) {
    disconnect(); // Disconnect serial connection
    key = 0; // Disable Processing from quiting when pressing ESC
  }
}

void controlEvent(ControlEvent theEvent) {
  if (theEvent.isGroup()) {
    if (theEvent.getGroup().getName() == dropdownList.getName())
      portNumber = int(theEvent.getGroup().getValue()); // Since the list returns a float, we need to convert it to an int. For that we us the int() function
  }
}

void connect() {
  disconnect(); // Disconnect any existing connection
  if (portNumber != -1) { // Check if com port and baudrate is set and if there is not already a connection established
    println("ConnectSerial");
    dropdownList.close();
    try {
      serial = new Serial(this, Serial.list()[portNumber], 57600);
    } catch (Exception e) {
      println("Couldn't open serial port");
      e.printStackTrace();
    }
    if (serial != null) {
      SaveSerialPort(Serial.list()[portNumber]);
      serial.bufferUntil('\n');
      connectedSerial = true;
      delay(3000); // Wait bit - needed for the standard serial connection, as it resets the board
      getAllValues();
      /*delay(50);
      serial.write("IB;"); // Start sending IMU values
      delay(50);
      serial.write("RB;"); // Start sending status report*/

    }
  } else if (portNumber == -1)
    println("Select COM Port first!");
  else if (connectedSerial)
    println("Already connected to a port!");
}

void dispose() { // Called automatically before shutting down
  disconnect();
}

void disconnect() {
  if (!connectedSerial)
    return;
  try {
    /*serial.write("IS;"); // Stop sending IMU values
    delay(10);
    serial.write("RS;"); // Stop sending status report
    delay(500);*/
    serial.stop();
    serial.clear(); // Empty the buffer
    connectedSerial = false;
    println("DisconnectSerial");
  } catch (Exception e) {
    //e.printStackTrace();
    println("Couldn't disconnect serial port");
  }
}

void SaveSerialPort(String port) {
    PrintWriter output = createWriter(portnameFile);
    output.print(port); // Write the comport to the file
    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
}
