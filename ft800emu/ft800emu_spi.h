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

	void begin();
	void end();

private:
	SPIClass(const SPIClass &);
	SPIClass &operator=(const SPIClass &);
	
}; /* class SPIClass */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SPI_H */

/* end of file */
