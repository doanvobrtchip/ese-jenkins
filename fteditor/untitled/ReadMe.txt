; Copyright (c) Future Technology Devices International 2015


; THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED ``AS IS'' AND ANY EXPRESS
; OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
; FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
; BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; THE POSSIBILITY OF SUCH DAMAGE.

Objective: 
==========
This generated folder directory contains the converted source from the editor as well as the necessary project dependencies to run on the the hardware and the FT8xx emulator.



Release package contents:
=========================
The folder structure is shown as below.
untitled
|	\---Bin
|	    \---Msvc_win32
|	    	\--libMPSSE.a - MPSSE library file for connecting to the hardware.
|	 
|	    \---MSVC_FT800Emu
|	        \--ft8xxemu.lib - FT8xx Emulator library
|	 	\---FT90x
|			\---libfatfs.a - ft90x fat file system library for the SD card.
|	\---Hdr
|	    \---FT_CoPro_Cmds.h  - Includes the FT8xx Coprocessor commands.
|	    \---FT_DataTypes.h - Includes the FT8xx datatypes.
|	    \---FT_Gpu.h - Includes the FT8xx commands.
|	    \---FT_Gpu_Hal.h - Includes the FT8xx HAL(Hardware Abstraction Layer) commands.
|	    \---FT_Hal_Utils.h - Includes the HAL utilities.
|	    \---FT_Platform.h - Includes Platform specific macros.
|		\---FT_ILI9488 - FTDI ILI9488 driver header file.
|	 
|	    \---Msvc_win32
|			\---FT_DataTypes.h - Data type for msvc_win32 platform.
|			\---FT_Platform.h - msvc_win32 platform configuration file. 
|	    	\--ftd2xx.h - ftd2xx header file
|	    	\--libMPSSE_spi.h - MPSSE SPI library header file
|	 
|	    \---Msvc_Emulator
|	    	\---FT_DataTypes.h - Data types used in the msvc_emulator HAL platform
|	    	\---FT_Emulator.h - Emulator tables and configuration header file.
|	        \---FT_Emulator_inttypes.h   - Data types used by the emulator.
|			\---FT_Emulator_main - The main function of the emulator project
|	        \---FT_Platform - The configuration file for the msvc_emulator HAL platform.
|
|
|   	\---FT90X
|  			\---\diskio.h
|  			\---\ff.h
|  			\---\ffconf.h
|  			\---\integer.h
|			\---\FT_DataTypes.h
|  			\---\FT_Platform.h
|
|	 
|	\---Project
|	    \---Msvc_win32 (MSVC project to run on an actual hardware)
|	        \---untitled 
|	            \---untitled.sln - MSVC project solution file.  Double click it will opens up the MSVC C++ IDE, if it's installed.
|	 		    \---untitled.vcxproj - MSVC project file for the IDE.
|	 			\---untitled.vcxproj.filter - MSVC project filter file for the IDE.
|	 
|	    \---Msvc_FT800Emu (MSVC project to run on an emulator)
|	        \---untitled 
|				\---Debug
|					\---ft8xxemu.dll - emulator run-time dependency.
|					\---SDL2.dll - Windows OpenGL wrapper. 
|				\---Release
|					\---ft8xxemu.dll - emulator run-time dependency.
|					\---SDL2.dll - Windows OpenGL wrapper. 
|	            \---untitled.sln - MSVC project solution file.  (FT800 emulator specific)Double click it will opens up the MSVC C++ IDE, if it's installed.
|	 		    \---untitled.vcxproj - MSVC project filter file for the IDE.
|	 			\---untitled.vcxproj.filter - MSVC project filter file for the IDE.
|
|	    \---FT90x (FT90x project to run on an actual hardware)
|	        \---untitled 
|	            \---.cproject - Eclipse .cproject file
|	 		    \---.project - Eclipse .project file
|
|	\---Src
|	    	\---FT_CoPro_Cmds.c - Coprocessor commands source file.
|	    	\---FT_Gpu_Hal.c - Gpu HAL source file.
|	    	\---FT_Emulator_main.cpp - The C wrapper of SPI/I2C interface of FT800 emulator and the main() function.
|	        \---untitled.c - The API implementation of FT800 sample application
|	 		\---FT_ILI9488.c - FTDI ILI9488 source driver.
|	\--Test - folder containing input test files such as .png .jpg, .raw etc.  For FT90x project, the contents have to be on the root level of the SD card.
|
|	\---Readme.txt - This file.
|
|---untitled.c - copy of the source file of the project.
|
|---(assets).h - copy of the embedded asset data, they can be deleted or be used in somewhere else.



Configuration Instructions:
===========================
This section contains details regarding various configurations supported by this software.

The configurations can be enabled/disabled via commenting/uncommenting macors in FT_Platform.h file. 
For MVSC/PC platform please look into .\untitled\Hdr\Msvc_win32\FT_Platform.h 
For arduino platform please look into .\untitled\Project\Arduino\untitled\FT_Platform.h
For FT90x platform please look into .\untitled\Hdr\FT90x\FT_Platform.h
For MVSC/PC Emulator platform please look into .\untitled\Hdr\MSVC_Emulator\FT_Platform.h 		
		
	

To run:
=========================
Unzip the package onto a respective project folder and open the solution/sketch file in the project folder and execute it. 
For MSVC/PC platform please execute .\untitled\Project\Msvc_win32\untitled\untitled.sln solution. 
For Arduino platform please execute.\untitled\Project\Arduino\untitled\untitled.ino sketch.
For MVSC/PC Emulator platform please execute .\untitled\Project\MSVC_Emulator\untitled\untitled.sln solution. 
For FT90x platform please import the project files in the .\untitled\Project\FT90x\untitled\ directory to the FTDI eclipse IDE.

The MSVC project file is compatible with Microsoft visual C++ version 2010.
The arduino project file is compatible with Arduino 1.0.5.
The MSVC emulator project is compatible with Microsoft Visual C++ version 2012 or above.


Reference Information:
======================
1. AN_245_VM800CB_SampleApp_PC_Introduction.pdf


Known issues and limitations:
=============================
1. MSVC(MPSSE) platform will work only on SPI mode.
2. The SPI host(Arduino, Windows PC) are assuming the data layout in memory as Little Endian format. 



