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
#include "ft800emu_inttypes.h"

// Project includes

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

	static void writeAddress(unsigned int address);
	static void writeByte(uint8_t data);

	static void readAddress(unsigned int address);
	static uint8_t readByte();

private:
	SPIClass(const SPIClass &);
	SPIClass &operator=(const SPIClass &);
	
}; /* class SPIClass */

extern SPIClass SPI;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SPI_H */

/* end of file */
