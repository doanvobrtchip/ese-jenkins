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
#include "ft8xxemu_inttypes.h"
#include <stdio.h>
#include <assert.h>

// Project includes

namespace FT800EMU {

/**
 * AudioProcessorClass
 * \brief AudioProcessorClass
 * \date 2013-11-03 09:45GMT
 */
class AudioProcessorClass
{
public:
  AudioProcessorClass() { }

  void begin();
  void end();

  int16_t execute(uint8_t &busy, uint16_t sound, uint8_t volume);
  void play();
	
private:
  int state;
  uint16_t acc;               // accumulator
  uint16_t cV[48], dV[48];    // code and data vectors
  uint16_t *pcV, *pdV;
}; /* class AudioProcessorClass */

extern AudioProcessorClass AudioProcessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_AUDIO_PROCESSOR_H */

/* end of file */

