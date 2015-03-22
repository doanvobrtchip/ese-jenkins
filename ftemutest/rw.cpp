/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <ft8xxemu.h>

uint8_t rd8(uint32_t address)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((address >> 16) & 0xFF);
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);
	FT8XXEMU_transfer(0x00);

	uint8_t value;
	value = FT8XXEMU_transfer(0);

	FT8XXEMU_cs(0);

	return value;
}

uint16_t rd16(uint32_t address)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((address >> 16) & 0xFF);
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);
	FT8XXEMU_transfer(0x00);

	uint16_t value;
	value = FT8XXEMU_transfer(0);
	value |= FT8XXEMU_transfer(0) << 8;

	FT8XXEMU_cs(0);
	return value;
}

uint32_t rd32(uint32_t address)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((address >> 16) & 0xFF);
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);
	FT8XXEMU_transfer(0x00);

	uint32_t value;
	value = FT8XXEMU_transfer(0);
	value |= FT8XXEMU_transfer(0) << 8;
	value |= FT8XXEMU_transfer(0) << 16;
	value |= FT8XXEMU_transfer(0) << 24;

	FT8XXEMU_cs(0);
	return value;
}

void wr8(uint32_t address, uint8_t value)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((2 << 6) | ((address >> 16) & 0xFF));
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);

	FT8XXEMU_transfer(value);

	FT8XXEMU_cs(0);
}

void wr16(uint32_t address, uint16_t value)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((2 << 6) | ((address >> 16) & 0xFF));
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);

	FT8XXEMU_transfer(value & 0xFF);
	FT8XXEMU_transfer((value >> 8) & 0xFF);

	FT8XXEMU_cs(0);
}

void wr32(uint32_t address, uint32_t value)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((2 << 6) | ((address >> 16) & 0xFF));
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);

	FT8XXEMU_transfer(value & 0xFF);
	FT8XXEMU_transfer((value >> 8) & 0xFF);
	FT8XXEMU_transfer((value >> 16) & 0xFF);
	FT8XXEMU_transfer((value >> 24) & 0xFF);

	FT8XXEMU_cs(0);
}

void wrstart(uint32_t address)
{
	FT8XXEMU_cs(1);

	FT8XXEMU_transfer((2 << 6) | ((address >> 16) & 0xFF));
	FT8XXEMU_transfer((address >> 8) & 0xFF);
	FT8XXEMU_transfer(address & 0xFF);
}

void wr8(uint8_t value)
{
	FT8XXEMU_transfer(value & 0xFF);
}

void wr16(uint16_t value)
{
	FT8XXEMU_transfer(value & 0xFF);
	FT8XXEMU_transfer((value >> 8) & 0xFF);
}
void wr32(uint32_t value)
{
	FT8XXEMU_transfer(value & 0xFF);
	FT8XXEMU_transfer((value >> 8) & 0xFF);
	FT8XXEMU_transfer((value >> 16) & 0xFF);
	FT8XXEMU_transfer((value >> 24) & 0xFF);
}

int wrstr(const char *str)
{
	int i = 0;
	for (char c = str[i]; c != 0; ++i, c = str[i])
	{
		FT8XXEMU_transfer(c);
	}
	FT8XXEMU_transfer(0);
	++i;
	int w = i;
	i %= 4;
	if (i)
	{
		i = 4 - i;
		w += i;
		for (int j = 0; j < i; ++j)
		{
			FT8XXEMU_transfer(0);
		}
	}
	return w;
}

void wrend()
{
	FT8XXEMU_cs(0);
}

/* end of file */
