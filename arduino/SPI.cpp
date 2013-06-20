/**
 * SPIClass
 * $Id$
 * \file SPI.cpp
 * \brief SPIClass
 */

/* 
 * Copyright (C) 2011  by authors
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

// #include <...>
#include "SPI.h"

// System includes

// Project includes
#include "ft800emu_ft800_spi.h"

// using namespace ...;

SPIClass SPI;

byte SPIClass::transfer(byte _data)
{
	return FT800EMU::FT800SPI.transfer(_data);
}

/* end of file */
