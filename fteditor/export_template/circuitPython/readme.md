### circuitPython support for Raspberry Pi Pico with Eve module from Bridgetek Pte Ltd
circuitPython firmware version used in this exported project: 0.1.2

### Usage
To run the exporting project, copy all file and folder to the drive "CIRCUITPY"

### Folder structure
--- lib
This folder contains the necessary files to use Eve device by Raspberry Pi Pico. 
It provides a python class to control Eve via SPI.

--- assets
This folder contains asset files which are actively used in ESE project

--- code.py
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
PICO GND   <--> Eve Board GND   

SD card is accessed from Eve SPI but with different CS pin:     
PICO GP13 <---> SD CS#

The connection with :     
PICO GP8 <---> D/CX pin of ILI9488  
PICO GP9 <---> CSX pin of ILI9488

### pico-brteve
This repo contains examples and library of Raspberry Pi Pico to support Bridgetek(brt) Eve, in C and circuitPython.
https://github.com/BRTSG-FOSS/pico-brteve

### Eve - Embedded Video Engine
An extreme easy-to-use yet powerful GPU with SPI interface to MCU. See more details:
https://brtchip.com/bt81x/
