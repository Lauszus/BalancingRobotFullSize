final byte SET_PID = 0;
final byte GET_PID = 1;
final byte SET_TARGET = 2;
final byte GET_TARGET = 3;
final byte SET_TURNING = 4;
final byte GET_TURNING = 5;
final byte SET_KALMAN = 6;
final byte GET_KALMAN = 7;

String commandHeader = "$S>"; // Standard command header
String responseHeader = "$S<"; // Standard response header

void sendCommand(byte output[]) {
  if (connectedSerial) {
    serial.write(commandHeader);
    serial.write(output);
    serial.write(getChecksum(output));
  } else
    println("Establish a serial connection first!");
}

// TODO: Only send the values that has actually changed
void setAllValues() {
  setPID(); // Set PID values
  delay(10);
  setTarget(); // Set target angle
  delay(10);
  setTurning(); // Set turning scale value
  delay(10);
  setKalman(); // Set Kalman values
  delay(10);
}

// TODO: Only get the values that has actually changed
void getAllValues() {
  getPID(); // Get PID values
  delay(10);
  getTarget(); // Get target angle
  delay(10);
  getTurning(); // Get turning scale value
  delay(10);
  getKalman(); // Get Kalman values
  delay(10);
}

void setPID() {
  if (!P.getText().isEmpty() && !I.getText().isEmpty() && !D.getText().isEmpty()) {
    int KpValue = (int)(Float.parseFloat(P.getText()) * 100.0); // All floats/doubles are multuplied by 100.0 before sending
    int KiValue = (int)(Float.parseFloat(I.getText()) * 100.0);
    int KdValue = (int)(Float.parseFloat(D.getText()) * 100.0);

    byte output[] = {
      SET_PID, // Cmd
      6, // Length
      (byte)(KpValue & 0xFF),
      (byte)(KpValue >> 8),
      (byte)(KiValue & 0xFF),
      (byte)(KiValue >> 8),
      (byte)(KdValue & 0xFF),
      (byte)(KdValue >> 8),
    };
    sendCommand(output); // Set PID values
  }
}

void getPID() {
  byte output[] = {
    GET_PID, // Cmd
    0, // Length
  };
  sendCommand(output); // Send output
}

void setTarget() {
  if (!targetAngle.getText().isEmpty()) {
    int targetAngleValue = (int)(Float.parseFloat(targetAngle.getText()) * 100.0); // All floats/doubles are multuplied by 100.0 before sending
    
    byte output[] = {
      SET_TARGET, // Cmd
      2, // Length
      (byte)(targetAngleValue & 0xFF),
      (byte)(targetAngleValue >> 8),
    };
    sendCommand(output); // Set PID values
  }
}

void getTarget() {
  byte output[] = {
    GET_TARGET, // Cmd
    0, // Length
  };
  sendCommand(output); // Send output
}

void setTurning() {
  if (!turnScale.getText().isEmpty()) {
    byte turningValue = Byte.parseByte(turnScale.getText());

    byte output[] = {
      SET_TURNING, // Cmd
      1, // Length
      (byte)(turningValue & 0xFF),
    };
    sendCommand(output); // Set PID values
  }
}

void getTurning() {
  byte output[] = {
    GET_TURNING, // Cmd
    0, // Length
  };
  sendCommand(output); // Send output
}

void setKalman() {
  if (!Qangle.getText().isEmpty() && !Qbias.getText().isEmpty() && !Rmeasure.getText().isEmpty()) {
    int QangleValue = (int)(Float.parseFloat(Qangle.getText()) * 10000.0); // The Kalman values are multiplied by 10000.0
    int QbiasValue = (int)(Float.parseFloat(Qbias.getText()) * 10000.0);
    int RmeasureValue = (int)(Float.parseFloat(Rmeasure.getText()) * 10000.0);

    byte output[] = {
      SET_KALMAN, // Cmd
      6, // Length
      (byte)(QangleValue & 0xFF),
      (byte)(QangleValue >> 8),
      (byte)(QbiasValue & 0xFF),
      (byte)(QbiasValue >> 8),
      (byte)(RmeasureValue & 0xFF),
      (byte)(RmeasureValue >> 8),
    };
    sendCommand(output); // Set PID values
  }
}

void getKalman() {
  byte output[] = {
    GET_KALMAN, // Cmd
    0, // Length
  };
  sendCommand(output); // Send output
}

byte getChecksum(byte data[]) {
  byte checksum = 0;
  for (int i = 0; i < data.length; i++)
    checksum ^= data[i];
  return checksum;
}

int getChecksum(int data[]) {
  int checksum = 0;
  for (int i = 0; i < data.length; i++)
    checksum ^= data[i];
  return checksum;
}
