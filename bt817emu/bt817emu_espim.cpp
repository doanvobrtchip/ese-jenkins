/*
BT817 Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "bt817emu_espim.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"
#include "ft800emu_memory.h"
#include <memory.h>
#include <algorithm>
#include <assert.h>

namespace BT817EMU {

#pragma warning(push)
#pragma warning(disable : 26495)
Espim::Espim(Memory *memory)
    : state(0)
    , m_Memory(memory)
{
	// ...
}
#pragma warning(pop)

int Espim::running()
{
	return state > 0;
}

void Espim::drive() // uint32_t spimi, uint32_t &spimo, uint32_t &spim_dir, uint32_t &spim_clken)
{
	uint8_t *ram = m_Memory->getRam();
	uint64_t *seq = (uint64_t *)(&ram[REG_ESPIM_SEQ]);

	int si = std::min(state - 1, 24);

	int opcode;
	if (si < 12)
		opcode = (seq[0] >> (5 * si)) & 0x1f;
	else if (si == 12)
		opcode = (((seq[1] & 1) << 4) + ((seq[0] >> 60) & 0xf)) & 0x1f;
	else
		opcode = (seq[1] >> ((5 * si) & 63)) & 0x1f;
	
	uint32_t spimo, spimoL, spim_dir, spim_clken;
	bool dtr = ram[REG_ESPIM_DTR] & 1;

	switch (opcode)
	{
		// clang-format off
	case SS_PAUSE:  spim_clken = 0; spim_dir = 0x1; spimo = 0;               spimoL = 0;                    break;
	case SS_S0:     spim_clken = 1; spim_dir = 0x1; spimo = 0;               spimoL = 0;                    break;
	case SS_S1:     spim_clken = 1; spim_dir = 0x1; spimo = 1;               spimoL = 1;                    break;
	case SS_A0:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 0) & 0xf;  spimoL = spimo;                break;
	case SS_A1:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 4) & 0xf;  spimoL = spimo;                break;
	case SS_A2:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 8) & 0xf;  spimoL = spimo;                break;
	case SS_A3:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 12) & 0xf; spimoL = spimo;                break;
	case SS_A4:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 16) & 0xf; spimoL = spimo;                break;
	case SS_A5:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 20) & 0xf; spimoL = spimo;                break;
	case SS_A6:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 24) & 0xf; spimoL = spimo;                break;
	case SS_A7:     spim_clken = 1; spim_dir = 0xf; spimo = (a >> 28) & 0xf; spimoL = spimo;                break;
	case SS_A76:    spim_clken = 1; spim_dir = 0xf; spimo = (a >> 28) & 0xf; spimoL = (a >> 24) & 0xf;      break;
	case SS_A54:    spim_clken = 1; spim_dir = 0xf; spimo = (a >> 20) & 0xf; spimoL = (a >> 16) & 0xf;      break;
	case SS_A32:    spim_clken = 1; spim_dir = 0xf; spimo = (a >> 12) & 0xf; spimoL = (a >> 8) & 0xf;       break;
	case SS_A10:    spim_clken = 1; spim_dir = 0xf; spimo = (a >> 4) & 0xf;  spimoL = (a >> 0) & 0xf;       break;
	case SS_QI:     spim_clken = 1; spim_dir = 0x0; spimo = 0;               spimoL = 0;                    break;
	default:        spim_clken = 1; spim_dir = 0xf; spimo = opcode & 0xf;    spimoL = (opcode ^ 0xf) & 0xf; break;
		// clang-format on
	}

	if (Memory::rawReadU32(ram, REG_SPIM_DIR) != (0x10 | spim_dir))
	{
		m_Memory->coprocessorWriteU32(REG_SPIM_DIR, 0x10 | spim_dir);
	}
	if (spim_clken)
	{
		// m_Memory->coprocessorWriteU32(REG_SPIM, 0x0 | spimo); // Not really needed for the emulator, but must work too!
		m_Memory->coprocessorWriteU32(REG_SPIM, 0x20 | spimo); // Rising edge, last read value (while still low) will be in spimi
		// m_Memory->coprocessorWriteU32(REG_SPIM, 0x20 | spimoL); // Not really needed for the emulator, but must work too!
		m_Memory->coprocessorWriteU32(REG_SPIM, spimoL); // Falling edge, last read value (while still high) will be in spimiL
	}
	else
	{
		// m_Memory->coprocessorWriteU32(REG_SPIM, 0x10 | spimo); // VERIFY: What's the usage of SS_PAUSE?
	}

	uint8_t *window = &ram[REG_ESPIM_WINDOW];
	int readstart = *(int *)(&ram[REG_ESPIM_READSTART]);
	// printf("[%3d]: opcode %x readstart=%d count=%d \n", state, opcode, readstart, count);

	int endpoint = readstart + (dtr ? 63 : 127);
	int lastcycle = (state == endpoint);

	// D7-D4 is in last low clock (spimi), D3-D0 is in last high clock (spimiL)
	uint32_t spimi = m_Memory->spimi(), spimiL = m_Memory->spimiL();
	if (!dtr)
		d[63] = (spimi) | (d[63] << 4);
	else
		d[63] = spimiL | (spimi << 4);

	if (!lastcycle)
	{
		state++;
	}
	else
	{
		memcpy(window, d, 64);
		Memory::rawWriteU32(ram, REG_ESPIM_TRIG, 1);
		if (--count)
		{
			state = readstart;
		}
		else
		{
			state = 0;
			m_Memory->coprocessorWriteU32(REG_SPIM, 0x10); // CS high
			m_Memory->coprocessorWriteU32(REG_SPIM_DIR, 0x11);
		}
	}
	assert(state < 256);

	if (dtr || ((state & 1) == (readstart & 1)))
		memmove(d, d + 1, 63);
}

void Espim::trigger()
{
	uint8_t *ram = m_Memory->getRam();
	state = 1;
	a = *(uint32_t *)(&ram[REG_ESPIM_ADD]);
	count = *(uint32_t *)(&ram[REG_ESPIM_COUNT]);
	// printf("trigger: a %x, count %i\n", a, count);
	Memory::rawWriteU32(ram, REG_ESPIM_TRIG, 0);
}

} /* namespace BT817EMU */

/* end of file */
