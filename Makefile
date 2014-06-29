# You should only have to change these values
PORT = /dev/tty.usbserial-AH00S1PI

# You should set the path to your Arduino application if it is not the default one
# The default for Linux and Solaris is:
#ARD_HOME = /opt/Arduino
# The default for Mac is:
#ARD_HOME = /Applications/Arduino.app

# Leave these alone
BOARD = pro
BOARD_SUB = pro.menu.cpu.16MHzatmega328
MON_SPEED = 57600

INC_DIRS = $(CURDIR)/KalmanFilter

include Arduino_Makefile_master/_Makefile.master