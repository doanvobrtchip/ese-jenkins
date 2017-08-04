/**
 * Emulator
 * $Id$
 * \file ft800emu_emulator.h
 * \brief Emulator
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

#ifndef BT8XXEMU_EMULATOR_H
#define BT8XXEMU_EMULATOR_H
namespace BT8XXEMU {

/**
* IEmulator
* \brief IEmulator
* \date 2011-08-04
* \author Jan Boon (Kaetemi)
*/
class IEmulator
{
public:
	virtual void stop() = 0;
	/*
	virtual uint8_t transfer(uint8_t data) = 0;
	virtual void cs(bool cs) = 0;
	virtual bool interrupt() = 0;

	virtual void touchSetXY(int idx, int x, int y, int pressure) = 0;
	virtual void touchResetXY(int idx) = 0;


	virtual uint8_t *getRam() = 0;
	// virtual uint8_t *getFlash() = 0;
	virtual const uint32_t *getDisplayList() = 0;
	virtual void poke() = 0;
	virtual int getDisplayListCoprocessorWrites() = 0;
	virtual void clearDisplayListCoprocessorWrites() = 0;
	virtual bool getDebugLimiterEffective() = 0;
	virtual int getDebugLimiterIndex() = 0;
	virtual void setDebugLimiter(int debugLimiter);
	virtual void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = 0;
	*/

};

}
#endif

namespace FT800EMU {

/**
 * Emulator
 * \brief Emulator
 * \date 2011-05-29 19:54GMT
 * \author Jan Boon (Kaetemi)
 */
class Emulator : public BT8XXEMU::IEmulator
{
public:
	Emulator() { }
	~Emulator() { }

	void run(const BT8XXEMU_EmulatorParameters &params);
	virtual void stop();

private:
	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

}; /* class Emulator */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_EMULATOR_H */

/* end of file */
