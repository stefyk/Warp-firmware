README.md document created by Stefany Kissovsky

This project implements a sensor system, measuring air quality parameters equivalent carbon dioxide (eCO2) and 
total volatile organic compounds (TVOC). This was achieved by interfacing CCS811 ultra-low power digital gas sensor 
with FRDM-KL03 development board by establishing I2C serial communication. The SSD1331 display was interfaced with the
FRDM-KL03 board by using SPI interface.

The wiring of the CCS811 sensor to the FRDM-KL03 is as follows:

Pin name on CCS811	Pin on board	Pin
I2C-SCL					PTB0		PIN 10
I2C-SDA					PTB1		PIN 9
VCC						5V			PIN5 LEFT SIDE
GND						GND			GND
Wake					GND			GND

The wiring of the SSD1331 sensor to the FRDM-KL03 is as follows:
Name of Pin	  GPIO pin on FRDMKLO3	 Pins on SSD1331
PinMOSI	   			PTA8	 				SI
PinSCK				PTA9					CK
PinCSn				PTB13					OC
PinDC				PTA12					DC
RST					PTB2					R
Ground				GND-7					G
Vcc					5V-5					+


The implementation and the build instructions for the system are the same as for the original Warp-firmware.
The core of the firmware is in warp-kl03-ksdk1.1-boot.c and when it is run, it initialize all required functions. 
The drivers for CCS811 and SSD1331 have been modified and two new files have been added for the display.
Here is a description of the files:

devCCS811.c 
It implements the function that detects human breath and estimates RR.

devCCS811.h
This is the header file for CCS811 and it has not been modified.

devSSD1331.c
This file implements the function that prints out separately each character on the OLED screen.

devSSD1331.h
This is the header file for SSD1331.

devtextSSD1331.c
This file was taken from https://os.mbed.com/users/star297/code/ssd1331/file/4385fd242db0/ssd1331.cpp/ and has been 
used for the functions defined in devSSD1331.c

devtextSSD1331.h
This file was taken from https://os.mbed.com/users/star297/code/ssd1331/file/4385fd242db0/ssd1331.h/
used for the functions defined in devSSD1331.c

