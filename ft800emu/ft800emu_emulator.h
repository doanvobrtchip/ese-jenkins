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
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"

namespace BT8XXEMU {

/**
* IEmulator
* \brief IEmulator
* \date 2011-08-04
* \author Jan Boon (Kaetemi)
*/
class IEmulator
{

	virtual void stop() = 0;
};

}

namespace FT800EMU {

/**
 * Emulator
 * \brief Emulator
 * \date 2011-05-29 19:54GMT
 * \author Jan Boon (Kaetemi)
 */
class Emulator
{
public:
	Emulator() { }
	~Emulator() { }

	void run(const BT8XXEMU_EmulatorParameters &params);
	void stop();

private:
	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

}; /* class Emulator */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
