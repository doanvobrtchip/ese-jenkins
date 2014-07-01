/**
 * EmulatorClass
 * $Id$
 * \file ft800emu_emulator.h
 * \brief EmulatorClass
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_EMULATOR_H
#define FT800EMU_EMULATOR_H
// #include <...>

// System includes
#include <string>

// Project includes (include standard stuff for user)
#include "ft800emu_inttypes.h"

namespace FT800EMU {

enum EmulatorMode
{
	EmulatorFT800 = 0,
	EmulatorFT801 = 1,
};

enum EmulatorFlags
{
	// enables the keyboard to be used as input
	EmulatorEnableKeyboard = 0x01,
	// enables audio
	EmulatorEnableAudio = 0x02,
	// enables coprocessor
	EmulatorEnableCoprocessor = 0x04,
	// enables mouse as touch
	EmulatorEnableMouse = 0x08,
	// enable debug shortkeys
	EmulatorEnableDebugShortkeys = 0x10,
	// enable graphics processor multithreading
	EmulatorEnableGraphicsMultithread = 0x20,
	// enable dynamic graphics quality degrading by interlacing
	EmulatorEnableDynamicDegrade = 0x40,
	// enable usage of REG_ROTATE
	EmulatorEnableRegRotate = 0x80,
	// enable emulating REG_PWM_DUTY by fading the rendered display to black
	EmulatorEnableRegPwmDutyEmulation = 0x100,
};

struct EmulatorParameters
{
public:
	EmulatorParameters() :
		Setup(0),
		Loop(0),
		Flags(0),
		Mode(EmulatorFT800),
		Keyboard(0),
		MousePressure(0),
		ExternalFrequency(0),
		ReduceGraphicsThreads(0),
		Graphics(0),
		//Interrupt(0)
		Exception(0),
		Close(0)
	{ }

	// Microcontroller function called before loop.
	void (*Setup)();
	// Microcontroller continuous loop.
	void (*Loop)();
	// See EmulatorFlags.
	int Flags;
	// Emulator mode
	EmulatorMode Mode;

	// Called after keyboard update.
	// Supplied function can use Keyboard.isKeyDown(FT800EMU_KEY_F3).
	void (*Keyboard)();
	// The default mouse pressure, default 0 (maximum).
	// See REG_TOUCH_RZTRESH, etc.
	uint32_t MousePressure;
	// External frequency. See CLK, etc.
	uint32_t ExternalFrequency;

	// Reduce graphics processor threads by specified number, default 0
	// Necessary when doing very heavy work on the MCU or Coprocessor
	// TODO: Maybe possible to automate this based on thread info
	uint32_t ReduceGraphicsThreads;

	// Replaces the default builtin ROM with a custom ROM from a file.
	std::string RomFilePath;
	// Replaces the builtin coprocessor ROM.
	std::string CoprocessorRomFilePath;

	// Graphics driverless mode
	// Setting this callback means no window will be created, and all
	// rendered graphics will be automatically sent to this function.
	// For enabling touch functionality, the functions
	// Memory.setTouchScreenXY and Memory.resetTouchScreenXY must be
	// called manually from the host application.
	// Builtin keyboard functionality is not supported and must be
	// implemented manually when using this mode.
	// The output parameter is false when the display is turned off.
	// The contents of the buffer pointer are undefined after this
	// function returns.
	// Return false when the application must exit.
	bool (*Graphics)(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize);

	// Interrupt handler
	//void (*Interrupt)();

	// Exception callback
	void (*Exception)(const char *message);

	// Safe exit
	void (*Close)();
};

/**
 * EmulatorClass
 * \brief EmulatorClass
 * \date 2011-05-29 19:54GMT
 * \author Jan Boon (Kaetemi)
 */
class EmulatorClass
{
public:
	EmulatorClass() { }

	static void run(const EmulatorParameters &params);
	static void stop();

private:
	EmulatorClass(const EmulatorClass &);
	EmulatorClass &operator=(const EmulatorClass &);

}; /* class EmulatorClass */

extern EmulatorClass Emulator;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
