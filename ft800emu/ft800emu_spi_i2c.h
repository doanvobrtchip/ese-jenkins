/**
 * SPII2CClass
 * $Id$
 * \file ft800emu_spi_i2c.h
 * \brief SPII2CClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_SPI_I2C_H
#define FT800EMU_SPI_I2C_H
// #include <...>

// System includes
#include <stdlib.h>

// Project includes
#include "ft8xxemu_inttypes.h"

namespace FT800EMU {
	class Memory;

/**
 * SPII2CClass
 * \brief SPII2CClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */
class SPII2CClass
{
public:
	SPII2CClass() { }

	static void begin(Memory *memory);
	static void end();

	static void csLow(int low = 1);
	static void csHigh(int high = 1);

	static uint8_t transfer(uint8_t data);

	// Interrupt
	static int intnLow();
	static int intnHigh();

private:
	SPII2CClass(const SPII2CClass &);
	SPII2CClass &operator=(const SPII2CClass &);

}; /* class SPII2CClass */

extern SPII2CClass SPII2C;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SPI_I2C_H */

/* end of file */
