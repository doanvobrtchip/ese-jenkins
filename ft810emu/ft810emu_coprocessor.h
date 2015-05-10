/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.h
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
 */

#ifdef FT810EMU_MODE
#ifndef FT800EMU_COPROCESSOR_H
#define FT800EMU_COPROCESSOR_H

// System includes
#include "ft8xxemu.h"
#include "ft8xxemu_inttypes.h"
#include <stdio.h>
#include <assert.h>

// Project includes

#define FT810EMU_COPROCESSOR_ROM_SIZE 16384

namespace FT810EMU {

typedef int16_t idct_t;

class Ejpg
{
private:
	size_t nbits;
	uint64_t bits;

	int state;
	uint8_t store[6 * 64];
	int bx, by;
	FILE *stim;
	size_t idct_i;
	int idct_phase;
	idct_t aa[64];
	size_t block;
	size_t blk_type;
	int16_t prevdc[4];
	uint16_t restart_i;
	int expect_ff;

	uint16_t rgb565(uint8_t y, int8_t Cb, int8_t Cr);

public:
	void reset();
	void startblock();
	void begin();
	void run2(uint8_t *memory8,
		uint16_t *memory16,
		uint32_t options,
		uint32_t dst,
		uint32_t w,
		uint32_t h,
		uint32_t format,
		uint32_t ri,
		uint32_t tq,
		uint32_t tda,
		uint32_t feed);
	void idct();
	void run(uint8_t *memory8,
		uint16_t *memory16,
		uint32_t options,
		uint32_t dst,
		uint32_t w,
		uint32_t h,
		uint32_t format,
		uint32_t ri,
		uint32_t qt,
		uint32_t tda,
		uint8_t  q[2 * 64],
		int bitsize,
		uint32_t feed);
	void run1(uint8_t *memory8,
		uint16_t *memory16,
		uint32_t options,
		uint32_t dst,
		uint32_t w,
		uint32_t h,
		uint32_t format,
		uint32_t ri,
		uint32_t qt,
		uint32_t tda,
		uint8_t  q[2 * 64],
		const char *btypes,
		uint8_t symbol,
		int16_t dm);

}; /* class EJpg */

/**
 * CoprocessorClass
 * \brief CoprocessorClass
 * \date 2013-08-03 02:10GMT
 */
class CoprocessorClass
{
public:
	CoprocessorClass() { }

	void begin(const char *romFilePath, FT8XXEMU_EmulatorMode mode);
	void end();

	void executeManual();
	void executeEmulator();

	void stopEmulator();

private:
	// template <bool singleFrame>
	void execute();

	void cpureset();

	void push(int v);
	int pop();

	CoprocessorClass(const CoprocessorClass &);
	CoprocessorClass &operator=(const CoprocessorClass &);

	volatile bool m_Running;

	uint32_t t;
	uint32_t d[32]; /* data stack */
	uint32_t r[32]; /* return stack */
	uint16_t pc;    /* program counter, counts CELLS */
	uint8_t dsp, rsp; /* point to top entry */
	uint32_t memrd;
	uint16_t j1boot[FT810EMU_COPROCESSOR_ROM_SIZE];

	Ejpg ejpg;

}; /* class CoprocessorClass */

extern CoprocessorClass Coprocessor;

} /* namespace FT810EMU */

#endif /* #ifndef FT800EMU_COPROCESSOR_H */
#endif /* #ifdef FT810EMU_MODE */

/* end of file */
