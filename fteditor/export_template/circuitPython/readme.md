# circuitPython support for Raspberry Pi Pico with Eve module from Bridgetek Pte Ltd

## lib
This folder contains the necessary files to use Eve device by Raspberry Pi Pico. 
It provides a python class to control Eve via SPI.

## assets
This folder contains asset files which are actively used in ESE project

## code.py
This file contains python script which is exported from ESE project

### Pico modules to work with Eve modules: 

##### MM2040EV 
##### ID2040

Both modules defines the same connections with EVE:  

PICO GP2 <--> Eve SCK   
PICO GP3 <--> Eve MOSI   
PICO GP4 <--> Eve MISO   
PICO GP5 <--> Eve CS#   
PICO GP7 <--> Eve PDN#   

PICO Power <--> Eve Board power   
PICO GND <--> Eve Board GND   

SD card is accessed from Eve SPI but with different CS pin:     
PICO GP13 <---> SD CS#

The connection with :     
PICO GP8 <---> D/CX pin of ILI9488  
PICO GP9 <---> CSX pin of ILI9488

### Eve - Embedded Video Engine
An extreme easy-to-use yet powerful GPU with SPI interface to MCU. See more details:
https://brtchip.com/bt81x/
