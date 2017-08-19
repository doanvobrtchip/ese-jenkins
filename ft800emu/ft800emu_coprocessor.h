/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.h
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
 */

#if defined(FT810EMU_MODE)
#include "ft810emu_coprocessor.h"
#else
#ifndef FT800EMU_COPROCESSOR_H
#define FT800EMU_COPROCESSOR_H

// System includes
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"
#include <stdio.h>
#include <assert.h>

// Project includes

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Memory;

/**
 * Coprocessor
 * \brief Coprocessor
 * \date 2013-08-03 02:10GMT
 */
class Coprocessor
{
public:
	Coprocessor(FT8XXEMU::System *system, Memory *memory, const char *romFilePath = 0, BT8XXEMU_EmulatorMode mode = BT8XXEMU_EmulatorFT800);
	~Coprocessor();

	void executeManual();
	void executeEmulator();

	void stopEmulator();

private:
	template <bool singleFrame>
	void execute();

private:
	FT8XXEMU::System *m_System;
	Memory *m_Memory;

	volatile bool m_Running;

	uint32_t t;
    uint32_t d[32]; /* data stack */
    uint32_t r[32]; /* return stack */
    uint16_t pc;    /* program counter, counts CELLS */
    uint8_t  dsp, rsp; /* point to top entry */

    void push(uint32_t v) { // push v on the data stack
        dsp = 31 & (dsp + 1);
        d[dsp] = t;
        t = v;
    }

    uint32_t pop(void) { // pop value from the data stack and return it
        uint32_t v = t;
        t = d[dsp];
        dsp = 31 & (dsp - 1);
        return v;
    }

private:
	Coprocessor(const Coprocessor &) = delete;
	Coprocessor &operator=(const Coprocessor &) = delete;

    /*int state;
    uint32_t acc;               // accumulator
    uint32_t cV[48], dV[48];    // code and data vectors
    uint32_t *pcV, *pdV;*/

}; /* class Coprocessor */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_COPROCESSOR_H */
#endif /* #ifndef FT810EMU_MODE */

/* end of file */
