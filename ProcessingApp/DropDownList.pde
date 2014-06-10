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

void initDropdownlist() {
  dropdownList = controlP5.addDropdownList("SerialPort"); // Make a dropdown list with all serial ports

  dropdownList.setPosition(10, 20);
  dropdownList.setSize(210, 200);
  dropdownList.setCaptionLabel("Select serial port"); // Set the lable of the bar when nothing is selected

  dropdownList.setBackgroundColor(color(200)); // Set the background color of the line between values
  dropdownList.setItemHeight(20); // Set the height of each item when the list is opened
  dropdownList.setBarHeight(15); // Set the height of the bar itself

  dropdownList.getCaptionLabel().getStyle().marginTop = 3; // Set the top margin of the lable
  dropdownList.getCaptionLabel().getStyle().marginLeft = 3; // Set the left margin of the lable

  dropdownList.setColorBackground(color(60));
  dropdownList.setColorActive(color(255, 128));

  BufferedReader reader = createReader(portnameFile);
  String line = null;
  try {
    line = reader.readLine();
    println("Serial port: " + line);
  } catch (IOException e) {
    e.printStackTrace();
  }

  // Now add the ports to the list, we use a for loop for that
  for (int i = 0; i < Serial.list().length; i++) {
    if (Serial.list()[i].indexOf("/dev/cu.") != -1)
      continue; // Do not display /dev/cu.* devices
    dropdownList.addItem(Serial.list()[i], i); // This is the line doing the actual adding of items, we use the current loop we are in to determine what place in the char array to access and what item number to add it as
    if (line != null && line.equals(Serial.list()[i]))
      dropdownList.setValue(i); // Automatically select the last selected serial port
  }

  addMouseWheelListener(new MouseWheelListener() { // Add a mousewheel listener to scroll the dropdown list
    public void mouseWheelMoved(MouseWheelEvent mwe) {
      dropdownList.scroll(mwe.getWheelRotation() > 0 ? 1 : 0); // Scroll the dropdownlist using the mousewheel
    }
  });

  controlP5.addButton("connect")
           .setPosition(225, 3)
           .setSize(45, 15);

  controlP5.addButton("disconnect")
           .setPosition(275, 3)
           .setSize(52, 15);
}
