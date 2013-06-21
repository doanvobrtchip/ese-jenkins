/**
 * FT800SPIClass
 * $Id$
 * \file ft800emu_ft800_spi.h
 * \brief FT800SPIClass
 * \date 2013-06-20 23:17GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_FT800_SPI_H
#define FT800EMU_FT800_SPI_H
// #include <...>

// System includes

// Project includes
#include "wiring.h"

namespace FT800EMU {

/**
 * FT800SPIClass
 * \brief FT800SPIClass
 * \date 2011-05-29 19:47GMT
 * \author Jan Boon (Kaetemi)
 */
class FT800SPIClass
{
public:
	FT800SPIClass() { }

	static void begin();
	static void end();

	static uint8_t *getRam();

private:
	FT800SPIClass(const FT800SPIClass &);
	FT800SPIClass &operator=(const FT800SPIClass &);

}; /* class FT800SPIClass */

extern FT800SPIClass FT800SPI;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_FT800_SPI_H */

/* end of file */
