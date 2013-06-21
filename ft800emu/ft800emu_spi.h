/**
 * SPIClass
 * $Id$
 * \file ft800emu_spi.h
 * \brief SPIClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_SPI_H
#define FT800EMU_SPI_H
// #include <...>

// System includes
#include <stdlib.h>

// Project includes
#include "ft800emu_inttypes.h"

namespace FT800EMU {

/**
 * SPIClass
 * \brief SPIClass
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */
class SPIClass
{
public:
	SPIClass() { }

	static void begin();
	static void end();

	static void csLow(bool low);
	static void csHigh(bool high);

	// uint8_t transfer(uint8_t data); // TODO SPI r/w data protocol...

	// temporary interface
	static void mcuSetAddress(size_t address);
	static void mcuWriteByte(uint8_t data);
	static uint8_t mcuReadByte();

private:
	SPIClass(const SPIClass &);
	SPIClass &operator=(const SPIClass &);
	
}; /* class SPIClass */

extern SPIClass SPI;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SPI_H */

/* end of file */
