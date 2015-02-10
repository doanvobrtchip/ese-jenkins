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

namespace FT800EMU {

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

	static void run(const FT8XXEMU_EmulatorParameters &params);
	static void stop();

private:
	EmulatorClass(const EmulatorClass &);
	EmulatorClass &operator=(const EmulatorClass &);

}; /* class EmulatorClass */

extern EmulatorClass Emulator;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
