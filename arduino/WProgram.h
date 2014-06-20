/**
 * WProgram
 * $Id$
 * \file WProgram.h
 * \brief WProgram
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

#ifndef WPROGRAM_H
#define WPROGRAM_H
// #include <...>

// System includes
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Project includes
#include "wiring.h"
#include "WString.h"
#include "DummySerial.h"

// Defines
#ifdef _MSC_VER
#	define and &&
#	define or ||
#endif /* #ifdef _MSC_VER */

// Functions
int32_t random(int32_t);
int32_t random(int32_t, int32_t);
void randomSeed(uint32_t);

#endif /* #ifndef WPROGRAM_H */

/* end of file */
