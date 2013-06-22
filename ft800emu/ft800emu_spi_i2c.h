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
#include "ft800emu_inttypes.h"

namespace FT800EMU {

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

	static void begin();
	static void end();

	static void csLow(bool low = true);
	static void csHigh(bool high = true);

	// uint8_t transfer(uint8_t data); // TODO SPI r/w data protocol...

	// temporary interface
	static void mcuSetAddress(size_t address);
	static void mcuWriteByte(uint8_t data);
	static uint8_t mcuReadByte();

private:
	SPII2CClass(const SPII2CClass &);
	SPII2CClass &operator=(const SPII2CClass &);
	
}; /* class SPII2CClass */

extern SPII2CClass SPII2C;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SPI_I2C_H */

/* end of file */
