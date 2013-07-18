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

namespace FT800EMU {

enum EmulatorFlags
{
	// enables the keyboard to be used as input
	EmulatorEnableKeyboard = 0x01,
	// enables audio
	EmulatorEnableAudio = 0x02,
};

struct EmulatorParameters
{
public:
	EmulatorParameters() : 
		Setup(0),
		Loop(0), 
		Flags(0),
		Keyboard(0) 
	{ }

	void (*Setup)(); // mcu function called before loop
	void (*Loop)(); // mcu continuous loop
	int Flags; // see EmulatorFlags
	
	// called after keyboard update, check Keyboard.isKeyDown(FT800EMU_KEY_F3)
	void (*Keyboard)();
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
