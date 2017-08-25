/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include <bt8xxemu.h>

BT8XXEMU_Emulator *g_Emulator = NULL;

uint8_t rd8(uint32_t address)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (address >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	BT8XXEMU_transfer(g_Emulator, 0x00);

	uint8_t value;
	value = BT8XXEMU_transfer(g_Emulator, 0);

	BT8XXEMU_cs(g_Emulator, 0);

	return value;
}

uint16_t rd16(uint32_t address)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (address >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	BT8XXEMU_transfer(g_Emulator, 0x00);

	uint16_t value;
	value = BT8XXEMU_transfer(g_Emulator, 0);
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 8;

	BT8XXEMU_cs(g_Emulator, 0);
	return value;
}

uint32_t rd32(uint32_t address)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (address >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
	BT8XXEMU_transfer(g_Emulator, 0x00);

	uint32_t value;
	value = BT8XXEMU_transfer(g_Emulator, 0);
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 8;
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 16;
	value |= BT8XXEMU_transfer(g_Emulator, 0) << 24;

	BT8XXEMU_cs(g_Emulator, 0);
	return value;
}

void wr8(uint32_t address, uint8_t value)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (2 << 6) | ((address >> 16) & 0xFF));
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);

	BT8XXEMU_transfer(g_Emulator, value);

	BT8XXEMU_cs(g_Emulator, 0);
}

void wr16(uint32_t address, uint16_t value)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (2 << 6) | ((address >> 16) & 0xFF));
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);

	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);

	BT8XXEMU_cs(g_Emulator, 0);
}

void wr32(uint32_t address, uint32_t value)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (2 << 6) | ((address >> 16) & 0xFF));
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);

	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 24) & 0xFF);

	BT8XXEMU_cs(g_Emulator, 0);
}

void wrstart(uint32_t address)
{
	BT8XXEMU_cs(g_Emulator, 1);

	BT8XXEMU_transfer(g_Emulator, (2 << 6) | ((address >> 16) & 0xFF));
	BT8XXEMU_transfer(g_Emulator, (address >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, address & 0xFF);
}

void wr8(uint8_t value)
{
	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
}

void wr16(uint16_t value)
{
	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);
}
void wr32(uint32_t value)
{
	BT8XXEMU_transfer(g_Emulator, value & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 8) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 16) & 0xFF);
	BT8XXEMU_transfer(g_Emulator, (value >> 24) & 0xFF);
}

int wrstr(const char *str)
{
	int i = 0;
	for (char c = str[i]; c != 0; ++i, c = str[i])
	{
		BT8XXEMU_transfer(g_Emulator, c);
	}
	BT8XXEMU_transfer(g_Emulator, 0);
	++i;
	int w = i;
	i %= 4;
	if (i)
	{
		i = 4 - i;
		w += i;
		for (int j = 0; j < i; ++j)
		{
			BT8XXEMU_transfer(g_Emulator, 0);
		}
	}
	return w;
}

void wrend()
{
	BT8XXEMU_cs(g_Emulator, 0);
}

/* end of file */
