/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef BT8XXEMU_PP_H
#define BT8XXEMU_PP_H

#include "bt8xxemu.h"

namespace BT8XXEMU {

class Emulator
{
public:
	virtual void stop() = 0;
	virtual bool isRunning() = 0;

	virtual uint8_t transfer(uint8_t data) = 0;
	virtual void cs(bool cs) = 0;
	virtual bool hasInterrupt() = 0;

	virtual void touchSetXY(int idx, int x, int y, int pressure) = 0;
	virtual void touchResetXY(int idx) = 0;

	virtual uint8_t *getRam() = 0;
	// virtual uint8_t *getFlash() = 0;
	virtual const uint32_t *getDisplayList() = 0;
	virtual void poke() = 0;
	virtual int *getDisplayListCoprocessorWrites() = 0;
	virtual void clearDisplayListCoprocessorWrites() = 0;

	virtual bool getDebugLimiterEffective() = 0;
	virtual int getDebugLimiterIndex() = 0;
	virtual void setDebugLimiter(int debugLimiter) = 0;
	virtual void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize) = 0;

};

}

#endif /* #ifndef BT8XXEMU_PP_H */

/* end of file */
