/*
BT815 Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Author: Jan Boon <jan@no-break.space>
*/

#include "bt815emu_espim.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"
#include "ft800emu_memory.h"
#include <memory.h>
#include <algorithm>
#include <assert.h>

namespace BT815EMU {

Espim::Espim(Memory *memory)  : state(0), m_Memory(memory)
{
	// ...
}

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

	uint32_t spimi = *(uint32_t *)(&ram[REG_SPIM]) & 0xf;
	uint32_t spimo, spim_dir, spim_clken;

	switch (opcode) {
	case SS_PAUSE:  spim_clken = 0; spim_dir = 0x1; spimo = 0;       break;
	case SS_S0:     spim_clken = 1; spim_dir = 0x1; spimo = 0;       break;
	case SS_S1:     spim_clken = 1; spim_dir = 0x1; spimo = 0;       break;
	case SS_A0:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 0;  break;
	case SS_A1:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 4;  break;
	case SS_A2:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 8;  break;
	case SS_A3:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 12; break;
	case SS_A4:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 16; break;
	case SS_A5:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 20; break;
	case SS_A6:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 24; break;
	case SS_A7:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 28; break;
	case SS_QI:     spim_clken = 1; spim_dir = 0x0; spimo = 0;       break;
	default:        spim_clken = 1; spim_dir = 0xf; spimo = opcode;  break;
	}
	spimo &= 0xf;

	if (Memory::rawReadU32(ram, REG_SPIM_DIR) != (0x10 | spim_dir))
	{
		m_Memory->coprocessorWriteU32(REG_SPIM_DIR, 0x10 | spim_dir);
	}
	if (spim_clken)
	{
		// m_Memory->coprocessorWriteU32(REG_SPIM, 0x0 | spimo);
		m_Memory->coprocessorWriteU32(REG_SPIM, 0x20 | spimo);
		m_Memory->coprocessorWriteU32(REG_SPIM, 0x0 | spimo);
	}
	else
	{
		// m_Memory->coprocessorWriteU32(REG_SPIM, 0x10 | spimo); // VERIFY
	}

	uint8_t *window = &ram[REG_ESPIM_WINDOW];
	int readstart = *(int *)(&ram[REG_ESPIM_READSTART]);
	// printf("[%3d]: opcode %x readstart=%d count=%d \n", state, opcode, readstart, count);

	int lastcycle = (state == (readstart + 127));

	d[63] = (spimi) | (d[63] << 4);
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
			m_Memory->coprocessorWriteU32(REG_SPIM, 0x10 | spimo); // CS high
		}
	}
	assert(state < 256);

	if ((state & 1) == (readstart & 1))
		memmove(d, d + 1, 63);
}

void Espim::trigger()
{
	uint8_t *ram = m_Memory->getRam();
	state = 1;
	a = *(uint32_t *)(&ram[REG_ESPIM_ADD]);
	count = *(uint32_t *)(&ram[REG_ESPIM_COUNT]);
	// printf("trigger: a %x, count %\n", a, count);
	Memory::rawWriteU32(ram, REG_ESPIM_TRIG, 0);
}

} /* namespace FT800EMU */

/* end of file */
