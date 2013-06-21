/**
 * wiring
 * $Id$
 * \file wiring.cpp
 * \brief wiring
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

#include <stdio.h>
#include "wiring.h"

// System includes

// Project includes
#include "ft800emu_system.h"
#include "ft800emu_ft800_spi.h"

// using namespace ...;


// Defines
#define FT800EMU_FT800_SS_PIN          9
#define FT800EMU_ARDUINO_INTERRUPT_PIN_0   2
#define FT800EMU_ARDUINO_INTERRUPT_PIN_1   3


// Digital pins
static uint8_t s_DigitalPins[64] = { 0 };
void pinMode(uint8_t pin, uint8_t mode) { if (mode == INPUT) s_DigitalPins[pin] = LOW; }

// Interrupts
static const int s_InterruptPins[2] = { FT800EMU_ARDUINO_INTERRUPT_PIN_0, FT800EMU_ARDUINO_INTERRUPT_PIN_1 };
static void (*s_InterruptFunctions[2])() = { 0 };
static int s_InterruptModes[2] = { 0 };


// Interrupts
static void ft800emuCallInterrupt(uint8_t i)
{
	bool isdt = FT800EMU::System.isMCUThread();
	if (!isdt) FT800EMU::System.holdMCUThread();
	s_InterruptFunctions[i]();
	if (!isdt) FT800EMU::System.resumeMCUThread();
}

static void ft800emuHandleInterrupt(uint8_t i, uint8_t val, uint8_t prev)
{
	if (s_InterruptFunctions[i])
	{
		switch (s_InterruptModes[i])
		{
		case LOW:
			if (val == LOW)
				ft800emuCallInterrupt(i);
			break;
		case CHANGE:
			if (val != prev)
				ft800emuCallInterrupt(i);
			break;
		case RISING:
			if (val == HIGH && prev == LOW)
				ft800emuCallInterrupt(i);
			break;
		case FALLING: 
			if (val == LOW && prev == HIGH)
				ft800emuCallInterrupt(i);
			break;
		}
	}
}

void attachInterrupt(uint8_t interrupt, void (*function)(void), int mode)
{
	s_InterruptModes[interrupt] = mode;
	s_InterruptFunctions[interrupt] = function;
}

void detachInterrupt(uint8_t interrupt)
{
	s_InterruptFunctions[interrupt] = NULL;
}


// Time
uint32_t millis(void) { return (uint32_t)FT800EMU::System.getMillis(); }
uint32_t micros(void) { return (uint32_t)FT800EMU::System.getMicros(); }
void delay(uint32_t ms) { FT800EMU::System.delay(ms); }
void delayMicroseconds(uint16_t us) { FT800EMU::System.delayMicros((int)us); }


// Digital pins
void digitalWrite(uint8_t pin, uint8_t val)
{
	val = val > 0 ? HIGH : LOW;
	uint8_t prev = s_DigitalPins[pin];
	s_DigitalPins[pin] = val;
	switch (pin)
	{
	case FT800EMU_FT800_SS_PIN:
		//FT800EMU::FT800SPI.slaveSelectChanged(val); // TODO
		break;
	case FT800EMU_ARDUINO_INTERRUPT_PIN_0:
		ft800emuHandleInterrupt(0, val, prev);
		break;
	case FT800EMU_ARDUINO_INTERRUPT_PIN_1:
		ft800emuHandleInterrupt(1, val, prev);
		break;
	}
}

uint8_t digitalRead(uint8_t pin)
{
	return s_DigitalPins[pin];
}


// Analog pins
int16_t analogRead(uint8_t pin)
{
	return FT800EMU::System.getAnalogRead(pin);
}

/* end of file */
