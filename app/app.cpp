/**
 * app.cpp
 * $Id$
 * \file app.cpp
 * \brief app.cpp
 * \date 2013-06-21 21:51GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <SPI.h>
#include <ft800emu_memory.h>
#include <vc.h>

void setup()
{
	FT800EMU::Memory.rawWriteU32(FT800EMU::Memory.getRam(), REG_PCLK, 5);
}

void loop()
{
	delay(10); // let's be nice on the cpu today :)
}
