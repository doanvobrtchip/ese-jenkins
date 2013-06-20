/**
 * DummySerial
 * $Id$
 * \file DummySerial.cpp
 * \brief DummySerial
 */

/* 
 * Copyright (C) 2011  Jan Boon (Kaetemi)
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
#include "DummySerial.h"

// System includes

// Project includes

// using namespace ...;

DummySerial Serial;

void DummySerial::write(uint8_t value)
{
	printf("%c", value);
}

/* end of file */
