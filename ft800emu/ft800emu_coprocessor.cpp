/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.cpp
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
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

CoprocessorClass Coprocessor;

static bool s_Running;

static const int sx[4] = { 0, 1, -2, -1 }; /* 2-bit sign extension */

static const uint16_t pgm_rom_ft800[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "resources/crom_ft800.h"
};
static const uint16_t pgm_rom_ft801[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "resources/crom_ft801.h"
};
static uint16_t pgm[FT800EMU_COPROCESSOR_ROM_SIZE];

void CoprocessorClass::begin(const char *romFilePath, FT8XXEMU_EmulatorMode mode)
{
	bool ft801 = (mode == FT8XXEMU_EmulatorFT801);
	if (romFilePath)
	{
		FILE *f;
		f = fopen(romFilePath, "rb");
		if (!f) printf("Failed to open coprocessor ROM file\n");
		else
		{
			size_t s = fread(pgm, 1, FT800EMU_COPROCESSOR_ROM_SIZE, f);
			if (s != FT800EMU_COPROCESSOR_ROM_SIZE) printf("Incomplete coprocessor ROM file\n");
			else printf("Loaded coprocessor ROM file\n");
			if (fclose(f)) printf("Error closing coprocessor ROM file\n");
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
void CoprocessorClass::execute()
{
	if (!singleFrame)
		s_Running = true;

    int _pc, _t, n;
    int insn;

    int swapped = 0;
    int starve = 0;
    do {
		if (Memory.coprocessorGetReset())
		{
			pc = 0;
			//printf("RESET COPROCESSOR\n");
			FT8XXEMU::System.delay(1);
			continue;
		}
        insn = pgm[pc];
        // printf("PC=%04x %04x\n", pc, insn);
        // if (pc == 0x1BA6) printf("COMMAND [%03x] %08x\n", MemoryClass::coprocessorReadU32(REG_CMD_READ), t);
		if (singleFrame)
		{
			if (pc == 0x0980) { // cmd.has1
                // 0x1090f8 is the location in coprocessor private RAM where the read pointer is cached.
				int rp = MemoryClass::coprocessorReadU32(0x1090f8);
				// printf("cmd.has1 %x %x\n", MemoryClass::coprocessorReadU32(REG_CMD_WRITE), rp);
				starve = MemoryClass::coprocessorReadU32(REG_CMD_WRITE) == rp;
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
                case 12:    _t = MemoryClass::coprocessorReadU32(t & ~3); /*printf("rd[%x] = %x\n", t, _t);*/ break;
                case 13:    _t = product; break;
                case 14:    _t = (n << 15) | (t & 0x7fff); break;
                case 15:    _t = -(n < t); break;
                case 16:    assert(0);
                case 17:    _t = n << t; break;
                case 18:    _t = MemoryClass::coprocessorReadU8(t); /*printf("rd8[%x] = %x\n", t, _t);*/ break;
                case 19:    _t = MemoryClass::coprocessorReadU16(t & ~1); break;
                case 20:    _t = product >> 32; break;
                case 21:    _t = product >> 16; break;
                case 22:    _t = t == r[rsp]; break;
                case 23:    _t = n - t; break;
                case 24:    _t = t + 1; break;
                case 25:    _t = (int16_t)MemoryClass::coprocessorReadU16(t); break;
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
					// printf("wr[%x] <= %x\n", t, n);
                    MemoryClass::coprocessorWriteU32(t, n);
					if (singleFrame)
					{
						if (t == REG_DLSWAP)
							swapped = 1;
					}
					break;
				case 5: // write16
                    MemoryClass::coprocessorWriteU16(t, n);
                    break;
                case 6: // write8
					MemoryClass::coprocessorWriteU8(t, n);
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
	} while (singleFrame ? (!swapped && !starve) : s_Running);
    // printf("coprocessor done\n");
}

void CoprocessorClass::executeManual()
{
	execute<true>();
}

void CoprocessorClass::executeEmulator()
{
	execute<false>();
}

void CoprocessorClass::stopEmulator()
{
	s_Running = false;
}

void CoprocessorClass::end()
{

}

} /* namespace GDEMU */
#endif /* #ifndef FT810EMU_MODE */

/* end of file */
