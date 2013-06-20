/**
 * DummySerial
 * $Id$
 * \file DummySerial.h
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

#ifndef FT800EMU_DUMMYSERIAL_H
#define FT800EMU_DUMMYSERIAL_H
// #include <...>

// System includes

// Project includes
#include "wiring.h"
#include "Stream.h"

/**
 * DummySerial
 * \brief DummySerial
 */
class DummySerial : public Stream
{
public:
	inline DummySerial() { }

    void begin(long) { }
    void end() { }
    virtual int available(void) { return -1; }
    virtual int peek(void) { return -1; }
    virtual int read(void) { return -1; }
    virtual void flush(void) { }
    virtual void write(uint8_t);
	
private:
	DummySerial(const DummySerial &);
	DummySerial &operator=(const DummySerial &);
	
}; /* class DummySerial */

extern DummySerial Serial;

#endif /* #ifndef FT800EMU_DUMMYSERIAL_H */

/* end of file */
