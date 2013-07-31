# ft800emu #

## Emulator ##

For an automatic emulator, call FT800EMU::Emulator.run(...) with the 
necessary initialization parameters. Make sure the platform pins are 
correctly wired up using the initialization parameters before calling 
run. 

### How to setup the emulator ###

A project will consist of 3 main parts, the application code, the 
ft800emu library, and a wrapper library for your microcontroller 
architecture. 

Inside the application code, create a main.cpp, which includes 
ft800emu_emulator.h, declares your application loop functions, and 
contains a main entry point function. 

    #include <ft800emu_emulator.h>

    void setup();
    void loop();

    int main(int, char* [])
    {
        FT800EMU::EmulatorParameters params;
        params.Setup = setup;
        params.Loop = loop;
        params.Flags = FT800EMU::EmulatorEnableKeyboard | FT800EMU::EmulatorEnableMouse | FT800EMU::EmulatorEnableDebugShortkeys;
        FT800EMU::Emulator.run(params);
        return 0;
    }

Add the ft800emu project and the microcontroller library wrapper as 
dependencies to your application project. 

That's all. 

### Emulator parameters ###

#### Setup ####

This is the function that your microcontroller would run when it 
launches, it is the same as the setup function in the arduino libraries. 

#### Loop ####

This function will be called continuously until the application exits. 

#### Keyboard ####

A user provided function called periodically, about 100 times per 
second, whenever new keyboard state is processed. You can use this to 
link digital pins on your microcontroller to keyboard keys, for example. 

    void keyboard()
	{
	    digitalWrite(4, FT800EMU::Keyboard.isKeydown(FT800EMU_KEY_A) ? LOW : HIGH);
	}

#### MousePressure ####

The value used as touch pressure when the left mouse button is pressed. 

#### Flags ####

##### EmulatorEnableKeyboard #####

Enables the keyboard functionality in the emulator. 

##### EmulatorEnableMouse #####

Enables the mouse cursor as touch input in the emulator. 

##### EmulatorEnableDebugShortkeys #####

Enables a number of debug shortkeys to help finding issues in an 
application. 

_F3_: Cycle through RGB, ALPHA, TAG and STENCIL display. 

_NUMPADPLUS_: Increase multiplier for display of ALPHA, TAG and STENCIL. 

_NUMPADMINUS_: Decrease multiplier. 

_NUMPADSLASH_: Reset multiplier to 1. 

_F8_: Increases limiter for number of displaylist commands to process. 

_F7_: Decreases displaylist limiter. 

_F6_: Disable the displaylist limiter. 

## Manual use ##

Write to the memory directly through the FT800EMU::Memory or 
FT800EMU::SPII2C interfaces. Create a display buffer in ARGB8888 format 
with the correct resolution as specified using REG_HSIZE and REG_VSIZE. 
Call FT800EMU::GraphicsProcessor.process(buffer, hsize, vsize) to build 
the display image. Before using a class, call the begin() function on it 
to initialize it, call end() after you're done. 

