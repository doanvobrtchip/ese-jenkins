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

// Project includes (include standard stuff for user)
#include "ft800emu_inttypes.h"

namespace FT800EMU {

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
};

struct EmulatorParameters
{
public:
	EmulatorParameters() : 
		Setup(0),
		Loop(0), 
		Flags(0),
		Keyboard(0), 
		MousePressure(0), 
		ExternalFrequency(0)
	{ }

	// Microcontroller function called before loop.
	void (*Setup)();
	// Microcontroller continuous loop.
	void (*Loop)();
	// See EmulatorFlags.
	int Flags;
	
	// Called after keyboard update.
	// Supplied function can use Keyboard.isKeyDown(FT800EMU_KEY_F3).
	void (*Keyboard)();
	// The default mouse pressure, default 0 (maximum).
	// See REG_TOUCH_RZTRESH, etc.
	uint32_t MousePressure;
	// External frequency. See CLK, etc.
	uint32_t ExternalFrequency;
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

private:
	EmulatorClass(const EmulatorClass &);
	EmulatorClass &operator=(const EmulatorClass &);

}; /* class EmulatorClass */

extern EmulatorClass Emulator;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
