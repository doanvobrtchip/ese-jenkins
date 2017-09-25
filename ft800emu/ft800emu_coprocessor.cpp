/*
FT800 Emulator Library
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
*/

#ifndef FT810EMU_MODE
#include "ft800emu_coprocessor.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft800emu_memory.h"
#include "ft8xxemu_system.h"
#include "ft800emu_vc.h"

// using namespace ...;

namespace FT800EMU {

#define FT800EMU_COPROCESSOR_ROM_SIZE 8192

static const int sx[4] = { 0, 1, -2, -1 }; /* 2-bit sign extension */

static const uint16_t pgm_rom_ft800[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "resources/crom_ft800.h"
};
static const uint16_t pgm_rom_ft801[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "resources/crom_ft801.h"
};
static uint16_t pgm[FT800EMU_COPROCESSOR_ROM_SIZE];

Coprocessor::Coprocessor(FT8XXEMU::System *system, Memory *memory, const wchar_t *romFilePath, BT8XXEMU_EmulatorMode mode)
	: m_System(system), m_Memory(memory)
{
	bool ft801 = (mode == BT8XXEMU_EmulatorFT801);
	if (romFilePath)
	{
		FILE *f;
		f = _wfopen(romFilePath, L"rb");
		if (!f) FTEMU_error("Failed to open coprocessor ROM file");
		else
		{
			size_t s = fread(pgm, 1, FT800EMU_COPROCESSOR_ROM_SIZE, f);
			if (s != FT800EMU_COPROCESSOR_ROM_SIZE) FTEMU_error("Incomplete coprocessor ROM file");
			else FTEMU_message("Loaded coprocessor ROM file");
			if (fclose(f)) FTEMU_error("Error closing coprocessor ROM file");
		}
	}
	else
	{
		if (ft801) memcpy(pgm, pgm_rom_ft801, sizeof(pgm_rom_ft801));
		else memcpy(pgm, pgm_rom_ft800, sizeof(pgm_rom_ft800));
	}

    pc = 0;
    dsp = 0;
    rsp = 0;
    t = 0;
}

template <bool singleFrame>
void Coprocessor::execute()
{
	if (!singleFrame)
		m_Running = true;

	uint16_t _pc;
	uint32_t _t, n;
	uint16_t insn;

    int swapped = 0;
    int starve = 0;
    do {
		if (m_Memory->coprocessorGetReset())
		{
			pc = 0;
			//FTEMU_printf("RESET COPROCESSOR\n");
			FT8XXEMU::System::delay(1);
			continue;
		}
        insn = pgm[pc];
        // FTEMU_printf("PC=%04x %04x\n", pc, insn);
        // if (pc == 0x1BA6) FTEMU_printf("COMMAND [%03x] %08x\n", m_Memory->coprocessorReadU32(REG_CMD_READ), t);
		if (singleFrame)
		{
			if (pc == 0x0980) { // cmd.has1
                // 0x1090f8 is the location in coprocessor private RAM where the read pointer is cached.
				int rp = m_Memory->coprocessorReadU32(0x1090f8);
				// FTEMU_printf("cmd.has1 %x %x\n", m_Memory->coprocessorReadU32(REG_CMD_WRITE), rp);
				starve = m_Memory->coprocessorReadU32(REG_CMD_WRITE) == rp;
			}
		}
        _pc = pc + 1;
        if (insn & 0x8000) { // literal
            push(insn & 0x7fff);
        } else {
			#define F8CP_TARGET (insn & 0x1fff)
			#define F8CP_INSN_6_4 (7 & (insn >> 4))

            switch (insn >> 13) {
            case 0: // jump
                _pc = F8CP_TARGET;
                break;
            case 1: // conditional jump
                if (pop() == 0)
                    _pc = F8CP_TARGET;
                break;
            case 2: // call
                rsp = 31 & (rsp + 1);
                r[rsp] = _pc << 1;
                _pc = F8CP_TARGET;
                break;
            case 3: // ALU
                if (insn & (1 << 7)) /* R->PC */
                    _pc = r[rsp] >> 1;
                n = d[dsp];
                uint64_t product = (int64_t)(int32_t)t * (int64_t)(int32_t)n;
                uint32_t sum32 = t + n;

                switch ((insn >> 8) & 31) {
                case 0:     _t = t; break;
                case 1:     _t = n; break;
                case 2:     _t = t + n; break;
                case 3:     _t = t & n; break;
                case 4:     _t = t | n; break;
                case 5:     _t = t ^ n; break;
                case 6:     _t = ~t; break;
                case 7:     _t = -(t == n); break;
                case 8:     _t = -((int32_t)n < (int32_t)t); break;
                case 9:     _t = n >> t; break;
                case 10:    _t = t - 1; break;
                case 11:    _t = r[rsp]; break;
                case 12:    _t = m_Memory->coprocessorReadU32(t & ~3); /*FTEMU_printf("rd[%x] = %x\n", t, _t);*/ break;
                case 13:    _t = product & 0xFFFFFFFF; break;
                case 14:    _t = (n << 15) | (t & 0x7fff); break;
                case 15:    _t = -(n < t); break;
                case 16:    assert(0);
                case 17:    _t = n << t; break;
                case 18:    _t = m_Memory->coprocessorReadU8(t); /*FTEMU_printf("rd8[%x] = %x\n", t, _t);*/ break;
                case 19:    _t = m_Memory->coprocessorReadU16(t & ~1); break;
                case 20:    _t = product >> 32; break;
				case 21:    _t = (product >> 16) & 0xFFFFFFFF; break;
                case 22:    _t = t == r[rsp]; break;
                case 23:    _t = n - t; break;
                case 24:    _t = t + 1; break;
                case 25:    _t = (int16_t)m_Memory->coprocessorReadU16(t); break;
                case 26:    _t = (sum32 & 0x80000000) ? 0 : ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); break;
                case 27:    _t = (0x00109 << 12) | (t & 0xfff); break;
                case 28:    _t = t + 2; break;
                case 29:    _t = t << 1; break;
                case 30:    _t = t + 4; break;
                case 31:    assert(0);
                }
                dsp = 31 & (dsp + sx[insn & 3]);
                rsp = 31 & (rsp + sx[(insn >> 2) & 3]);

                switch (F8CP_INSN_6_4)
                {
				case 1: // T_N
					d[dsp] = t;
					break;
				case 2: // T_R
					r[rsp] = t;
					break;
				case 3: // Rinc
                    ++r[rsp];
                    break;
				case 4: // write32
					// FTEMU_printf("wr[%x] <= %x\n", t, n);
                    m_Memory->coprocessorWriteU32(t, n);
					if (singleFrame)
					{
						if (t == REG_DLSWAP)
							swapped = 1;
					}
					break;
				case 5: // write16
                    m_Memory->coprocessorWriteU16(t, n);
                    break;
                case 6: // write8
					m_Memory->coprocessorWriteU8(t, n);
					break;
				case 7: // read
					// no-op
					break;
				}

                t = _t;
                break;
            }
        }
        pc = _pc;
        // fflush(stdout);
	} while (singleFrame ? (!swapped && !starve) : m_Running);
    // FTEMU_printf("coprocessor done\n");
}

void Coprocessor::executeManual()
{
	execute<true>();
}

void Coprocessor::executeEmulator()
{
	execute<false>();
}

void Coprocessor::stopEmulator()
{
	m_Running = false;
}

Coprocessor::~Coprocessor()
{

}

} /* namespace GDEMU */
#endif /* #ifndef FT810EMU_MODE */

/* end of file */
