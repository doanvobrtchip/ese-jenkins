# Introduction
========

### Projects
This repository contains the following projects:  

EVE Emulator Library  

EVE Screen Editor

#### EVE Emulator Library
The latest version of EVE emulator library supports the following chip: 

FT80X, FT81X, BT81X.  

To emulating BT81X, the flash chip "MX25L" is emulated. Therefore, the "mx25lemu.dll" 
is required to work with flash features of BT81X chip. 


##### DLL mode library
bt8xxemu.dll is the executable. The project is linked with bt815emu.lib , ft8xxemu_platform.lib  as well as astc.lib.
In addition, to work with flash feature, the flash emulator "mx25lemu.dll" is required. 

This DLL supports all the EVE chips and the different chip can be selected at run-time. 
##### Service mode library
bt8xxemus.exe is the executable but it loads bt8xxemu.dll. It provides a background service of EVE emulator so that 
several EVE chips can be emulated at the same time. 

#### EVE Screen Editor (ESE)
EVE screen Editor is one GUI application based Qt but powered by EVE emulator library.   

It provides users an intuitive UI to explore the features of EVE without EVE hardware required.

In addition, user may get the working code for BridgeTek EVE based modules by exporting ESE the project.  

The source code is under "fteditor" folder and final executable is "fteditor.exe". 


