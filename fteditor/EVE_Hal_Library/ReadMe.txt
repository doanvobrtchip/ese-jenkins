; Copyright (c) Bridgetek Pte Ltd.
; THIS SOFTWARE IS PROVIDED BY BRIDGETEK PTE LTD. ``AS IS'' AND ANY EXPRESS
; OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
; FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
; BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
; BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; THE POSSIBILITY OF SUCH DAMAGE.



Objective: 
==========
This generated folder directory contains the converted source from the editor
as well as the necessary project dependencies to run on the the hardware and
the BT8xx emulator.

Release package contents:
=========================
The folder structure is shown as below.


+---Bin
|    \---Msvc
|        \--libMPSSE.a - MPSSE library file.
|        \--ftd2xx.lib - FTD2XX library.
|        \--LibFT4222.lib - FT4222 library file.
|    \---Msvc_Emulator
|        \--ft800emu.lib - MPSSE library file.|
|        \--ft800emud.lib - MPSSE library file.|
|
+---Hdr
|    \---Msvc
|        \---\ftd2xx.h - ftd2xx header file
|        \---\libMPSSE_spi - MPSSE header file
|        \---\Platform.h - Includes Platform specific macros.
|        \---\LibFT4222.h - LibFT4222 header file 
|    \---Msvc_Emulator
|        \---\FT800emu_emulator.h
|        \---\FT800Emu_main.h
|        \---\FT800emu_spi_i2c.h
|        \---\Platform.h - Includes Platform specific macros.
|    \---FT90x
|        \---\FT800emu_emulator.h
|        \---\FT800Emu_main.h
|        \---\FT800emu_spi_i2c.h
|        \---\Platform.h - Includes Platform specific macros.
|    \---\Assets.h - Includes content files.
|    \---\CoPro_Cmds.h - Includes the Coprocessor commands.
|    \---\Gpu.h - Includes the GPU commands.
|    \---\Gpu_Hal.h - Includes the GPU HAL commands.
|    \---\Hal_Config.h - Configurations for Hardware.
|    \---\Hal_Utils.h - Includes the HAL utilities.
|    \---\App_Common.h - Includes common application APIs
|    \---\ILI9488.h - Includes ILI9488 driver header
|
+---Src
|    \---FT90x
|        \---fatfs - Fatfs library source
|        \---\Gpu_Hal.c - Gpu hal source commands file.
|    \---FT93x
|        \---fatfs - Fatfs library source
|        \---\Gpu_Hal.c - Gpu hal source commands file.
|    \--- Msvc_Emulator
|        \---\Emu_main.cpp
|        \---\Gpu_Hal.c - Gpu hal source commands file.
|    \--- Msvc
|        \---\Gpu_Hal.c - Gpu hal source commands file.
|    \---\CoPro_Cmds.c - Coprocessor commands source file.
|    \---\Hal_Utils.c - Math library
|    \---\App_Common.c - Common APIs source file.
|    \---\ILI9488.c - ILI9488 driver source
|    \---\Skeleton.c - exporting commands of the project
|
+---Project
|    \---FT90x
|        \---\.cproject - Eclipse project settings
|        \---\.project - Eclipse project configurations
|    \---FT93x
|        \---\.cproject - Eclipse project settings
|        \---\.project - Eclipse project configurations
|    \---Msvc
|        \---\<App_Name>.sln - Visual studio solution
|        \---\<App_Name>.vcxproj - Visual studio project
|    \---Msvc_Emulator
|        \---\<App_Name>.sln - Visual studio solution
|        \---\<App_Name>.vcxproj - Visual studio project
|
+--Test - folder containing input test files such as .wav, .jpg, .raw etc


Configuration Instructions:
===========================
This section contains details regarding various configurations supported by this software.

The configurations can be enabled/disabled via commenting/uncommenting macros in Platform.h file. 
For MVSC/PC platform please look into .\Hdr\Msvc\Platform.h 
For FT90x platform please look into .\Hdr\FT90x\Platform.h
For FT93x platform please look into .\Hdr\FT93x\Platform.h
For MVSC/PC Emulator platform please look into .\Hdr\MSVC_Emulator\Platform.h 

Reference Information:
======================
Please refer to BT81X_Programmer_Guide for more information on programming.

Known issues:
=============
N.A

Limitations:
============
N.A

Extra Information:
==================
N.A


