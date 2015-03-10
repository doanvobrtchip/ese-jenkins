/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.h
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
 */

#ifdef FT810EMU_MODE
#include "ft810emu_coprocessor.h"
#endif
#ifndef FT810EMU_MODE
#ifndef FT800EMU_COPROCESSOR_H
#define FT800EMU_COPROCESSOR_H

// System includes
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"
#include <stdio.h>
#include <assert.h>

// Project includes

namespace FT800EMU {

/**
 * CoprocessorClass
 * \brief CoprocessorClass
 * \date 2013-08-03 02:10GMT
 */
class CoprocessorClass
{
public:
	CoprocessorClass() { }

	void begin(const char *romFilePath = 0, FT8XXEMU_EmulatorMode mode = FT8XXEMU_EmulatorFT800);
	void end();

	void executeManual();
	void executeEmulator();

	void stopEmulator();

private:
	template <bool singleFrame>
	void execute();

	CoprocessorClass(const CoprocessorClass &);
	CoprocessorClass &operator=(const CoprocessorClass &);

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

    /*int state;
    uint32_t acc;               // accumulator
    uint32_t cV[48], dV[48];    // code and data vectors
    uint32_t *pcV, *pdV;*/

}; /* class CoprocessorClass */

extern CoprocessorClass Coprocessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_COPROCESSOR_H */
#endif /* #ifndef FT810EMU_MODE */

/* end of file */
