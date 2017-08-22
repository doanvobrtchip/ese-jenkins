/**
 * audio processor
 * $Id$
 * \file ft800emu_coprocessor.h
 * \brief graphics coprocessor
 * \date 2013-11-03 09:45GMT
 */

#ifndef FT800EMU_AUDIO_PROCESSOR_H
#define FT800EMU_AUDIO_PROCESSOR_H

// System includes
#include "bt8xxemu_inttypes.h"

// Project includes

#define FT800EMU_AUDIO_MEMSIZE 40458

namespace FT800EMU {

/**
 * AudioProcessor
 * \brief AudioProcessor
 * \date 2013-11-03 09:45GMT
 */
class AudioProcessor
{
public:
	AudioProcessor();
	~AudioProcessor();

	int16_t execute(uint8_t &busy, uint16_t sound, uint8_t volume);
	void play();
	
private:
	uint16_t mem[FT800EMU_AUDIO_MEMSIZE];
	int state;
	uint16_t acc;               // accumulator
	uint16_t cV[48], dV[48];    // code and data vectors
	uint16_t *pcV, *pdV;

}; /* class AudioProcessor */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_AUDIO_PROCESSOR_H */

/* end of file */

