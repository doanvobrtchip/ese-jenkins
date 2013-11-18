/**
 * AudioRenderClass
 * $Id$
 * \file ft800emu_audio_render.cpp
 * \brief AudioRenderClass
 * \date 2013-10-17 23:44GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft800emu_audio_render.h"

// System includes
#include <stdio.h>

// Project includes
#include "ft800emu_system.h"
#include "ft800emu_audio_driver.h"
#include "ft800emu_audio_processor.h"
#include "ft800emu_memory.h"
#include "vc.h"

// using namespace ...;

namespace FT800EMU {

AudioRenderClass AudioRender;

// Simple nearest resampling
#define FT800EMU_SYNTH_RATE 48000
static double s_SecondsPerSample = 1.0 / (double)FT800EMU_SYNTH_RATE;
static double s_SecondsPassedForSample = 0.0;
static short s_Sample = 0;

void AudioRenderClass::process()
{
	short *audioBuffer;
	int samples;

	AudioDriver.beginBuffer(&audioBuffer, &samples);

	if (audioBuffer != NULL)
	{
		process(audioBuffer, samples);
		AudioDriver.endBuffer();
	}
}

void AudioRenderClass::process(short *audioBuffer, int samples)
{
	// printf("process audio\n");
	
	int16_t synth;
	int16_t sample = s_Sample;
	uint8_t *ram = Memory.getRam();
	uint8_t &busy = ram[REG_PLAY];
	uint16_t &sound = *static_cast<uint16_t *>(static_cast<void *>(&ram[REG_SOUND]));
	uint8_t &volume = ram[REG_VOL_SOUND];
	int audioFrequency = AudioDriver.getFrequency();
	double secondsPerSample = 1.0 / (double)audioFrequency;
	int channels = AudioDriver.getChannels();
	for (int i = 0; i < samples; ++i)
	{
		if (audioFrequency != FT800EMU_SYNTH_RATE)
		{
BeginSample:
			if (s_SecondsPassedForSample < s_SecondsPerSample)
				goto EndSample;
			s_SecondsPassedForSample -= s_SecondsPerSample;
		}
		
		synth = AudioProcessor.execute(busy, sound, volume);
		
		// todo: ram buffer playback and proper resampling
		
		sample = synth;
		if (audioFrequency != FT800EMU_SYNTH_RATE)
		{
			goto BeginSample;
		}
EndSample:
		for (int j = 0; j < channels; ++j) // just fill all channels for now
		{
			audioBuffer[(i * channels) + j] = sample;
		}
		if (audioFrequency != FT800EMU_SYNTH_RATE)
		{
			s_SecondsPassedForSample += secondsPerSample;
		}
	}
	s_Sample = sample;
}

} /* namespace FT800EMU */

/* end of file */
