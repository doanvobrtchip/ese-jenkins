/**
 * AudioRender
 * $Id$
 * \file ft800emu_audio_render.h
 * \brief AudioRender
 * \date 2013-10-17 23:44GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_AUDIO_RENDER_H
#define FT800EMU_AUDIO_RENDER_H
#include "ft8xxemu_inttypes.h"

// System includes
#include <stdio.h>

// Project includes

// Simple linear resampling
#define FT800EMU_SYNTH_RATE 48000

namespace FT8XXEMU {
	class AudioOutput;
}

namespace FT800EMU {
	class Memory;
	class AudioProcessor;

/**
 * AudioRender
 * \brief AudioRender
 * \date 2013-10-17 23:44GMT
 * \author Jan Boon (Kaetemi)
 */
class AudioRender
{
public:
	AudioRender(FT8XXEMU::AudioOutput *audioOutput, Memory *memory, AudioProcessor *audioProcessor);
	~AudioRender();

	void playbackPlay();
	void process();
	void process(short *audioBuffer, int samples);

private:
	BT8XXEMU_FORCE_INLINE int16_t playback(uint32_t &playbackStart,
		uint32_t &playbackLength, uint8_t &playbackFormat,
		uint8_t &playbackLoop, uint8_t &playbackBusy,
		uint32_t &playbackReadPtr, uint8_t &playbackVolume,
		bool &adpcmNext, uint8_t *ram);

private:
	double m_SecondsPerSynthSample = 1.0 / (double)FT800EMU_SYNTH_RATE;
	double m_SecondsPassedForSynthSample = 0.0;
	short m_SynthSample0 = 0;
	short m_SynthSample1 = 0;
	double m_SecondsPassedForPlaybackSample = 0.0;
	short m_PlaybackSample0 = 0;
	short m_PlaybackSample1 = 0;
	bool m_ADPCMNext = false;

	int m_ADPCMPredictedSample = 0;
	int m_ADPCMIndex = 0;

	bool m_RequestPlayback = false;

	FT8XXEMU::AudioOutput *m_AudioOutput = NULL;
	Memory *m_Memory = NULL;
	AudioProcessor *m_AudioProcessor = NULL;

private:
	AudioRender(const AudioRender &) = delete;
	AudioRender &operator=(const AudioRender &) = delete;

}; /* class AudioRender */

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_AUDIO_RENDER_H */

/* end of file */
