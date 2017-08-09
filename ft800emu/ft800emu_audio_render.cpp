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
#include "ft8xxemu_system.h"
#include "ft8xxemu_audio_output.h"
#include "ft800emu_audio_processor.h"
#include "ft800emu_memory.h"
#include "ft8xxemu_minmax.h"
#include "ft800emu_vc.h"

// using namespace ...;

namespace FT800EMU {

AudioRenderClass AudioRender;

// Simple linear resampling
#define FT800EMU_SYNTH_RATE 48000
static double s_SecondsPerSynthSample = 1.0 / (double)FT800EMU_SYNTH_RATE;
static double s_SecondsPassedForSynthSample = 0.0;
static short s_SynthSample0 = 0;
static short s_SynthSample1 = 0;
static double s_SecondsPassedForPlaybackSample = 0.0;
static short s_PlaybackSample0 = 0;
static short s_PlaybackSample1 = 0;
static bool s_ADPCMNext = false;

static bool s_RequestPlayback = false;

static FT8XXEMU::AudioOutput *s_AudioOutput = NULL;

void AudioRenderClass::begin(FT8XXEMU::AudioOutput *audioOutput)
{
	s_AudioOutput = audioOutput;
	s_AudioOutput->onAudioProcess([](short *audioBuffer, int samples) -> void {
		process(audioBuffer, samples);
	});
}

void AudioRenderClass::end()
{

}

void AudioRenderClass::playbackPlay()
{
	s_RequestPlayback = true;
}

static int16_t ulawranges[] = {
	-8159,
	-4063,
	-2015,
	-991,
	-479,
	-223,
	-95,
	-31,
	8158,
	4062,
	2014,
	990,
	478,
	222,
	94,
	30
};

static int s_ADPCMPredictedSample = 0;
static int s_ADPCMIndex = 0;
static int s_ADPCMIndexTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};
static int s_ADPCMStepsizeTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14,
	16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60,
	66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209,
	230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878,
	2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
	5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635,
	13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

static BT8XXEMU_FORCE_INLINE int16_t playback(uint32_t &playbackStart,
	uint32_t &playbackLength, uint8_t &playbackFormat,
	uint8_t &playbackLoop, uint8_t &playbackBusy,
	uint32_t &playbackReadPtr, uint8_t &playbackVolume,
	bool &adpcmNext, uint8_t *ram)
{
	if (s_RequestPlayback)
	{
		s_RequestPlayback = false;
		playbackBusy = 1;
		playbackReadPtr = playbackStart;
		adpcmNext = false;
		s_ADPCMPredictedSample = 0;
		s_ADPCMIndex = 0;
	}
	if (playbackBusy)
	{
		if (!adpcmNext) // don't increase on second half of adpcm byte
		{
			++playbackReadPtr;
			if (playbackReadPtr > (playbackStart + playbackLength))
			{
				if (playbackLoop & 0x01)
				{
					playbackReadPtr = playbackStart;
				}
				else
				{
					playbackBusy = 0;
					return 0;
				}
			}
		}
		switch (playbackFormat)
		{
			case 0: // PCM
			{
				int8_t value = *static_cast<int8_t *>(static_cast<void *>(&ram[playbackReadPtr]));
				int16_t result = (int16_t)value * (int16_t)playbackVolume;
				return result;
			}
			case 1: // µLaw
			{
				uint8_t value = ram[playbackReadPtr];
				switch (value)
				{
					case 0xFF:
						return 0;
					case 0x7F:
						return -1;
				}
				uint8_t sign = value & (0x80);
				int16_t range = (int16_t)(uint8_t)((value & (0x70)) >> 4);
				int16_t step = (int16_t)(uint8_t)(value & (0x0F));
				int16_t stepm = step << (8 - range); // 0 becomes 256, 7=2
				int16_t rangem = ulawranges[(value & (0xF0)) >> 4];
				if (sign) stepm = -stepm;
				int16_t result = rangem + stepm;
				int32_t resultv = (int32_t)result * (int32_t)playbackVolume;
				return (int16_t)(resultv >> 8);
			}
			case 2: // ADPCM
			{
				// See: http://www.cs.columbia.edu/~hgs/audio/dvi/
				// Adapted from public domain IMA reference implementation

				uint8_t originalSample = ram[playbackReadPtr];
				if (adpcmNext) originalSample = originalSample >> 4;
				else originalSample = originalSample & 0x0F;
				adpcmNext = !adpcmNext;

				/* compute predicted sample estimate newSample */
				/* calculate difference = (originalSample + 1⁄2) * stepsize/4: */
				int stepSize = s_ADPCMStepsizeTable[s_ADPCMIndex];
				int difference = 0;
				if (originalSample & 4) /* perform multiplication through repetitive addition */
					difference += stepSize;
				if (originalSample & 2)
					difference += stepSize >> 1;
				if (originalSample & 1)
					difference += stepSize >> 2;

				/* (originalSample + 1⁄2) * stepsize/4 =originalSample * stepsize/4 + stepsize/8: */
				difference += stepSize >> 3;

				if (originalSample & 8) /* account for sign bit */
					difference = -difference;

				/* adjust predicted sample based on calculated difference: */
				s_ADPCMPredictedSample += difference;
				s_ADPCMPredictedSample = min(max(-32768, s_ADPCMPredictedSample), 32767); /* check for overflow */

				/* compute new stepsize */
				/*adjust index into stepsize lookup table using originalSample: */
				s_ADPCMIndex += s_ADPCMIndexTable[originalSample];
				s_ADPCMIndex = min(max(0, s_ADPCMIndex), 88); /* check for overflow */

				return (int16_t)((s_ADPCMPredictedSample * (int32_t)playbackVolume) >> 8);
			}
			default:
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}
}

void AudioRenderClass::process()
{
	short *audioBuffer;
	int samples;

	s_AudioOutput->beginBuffer(&audioBuffer, &samples);

	if (audioBuffer != NULL)
	{
		process(audioBuffer, samples);
		s_AudioOutput->endBuffer();
	}
}

void AudioRenderClass::process(short *audioBuffer, int samples)
{
	// FTEMU_printf("process audio\n");
	uint8_t *ram = Memory.getRam();

	int16_t synthSample0 = s_SynthSample0;
	int16_t synthSample1 = s_SynthSample1;

	int16_t playbackSample0 = s_PlaybackSample0;
	int16_t playbackSample1 = s_PlaybackSample1;

	uint8_t &busy = ram[REG_PLAY];
	uint16_t &sound = *static_cast<uint16_t *>(static_cast<void *>(&ram[REG_SOUND]));
	uint8_t &volume = ram[REG_VOL_SOUND];

	uint32_t &playbackStart = *static_cast<uint32_t *>(static_cast<void *>(&ram[REG_PLAYBACK_START])); playbackStart &= FT800EMU_ADDR_MASK;
	uint32_t &playbackLength = *static_cast<uint32_t *>(static_cast<void *>(&ram[REG_PLAYBACK_LENGTH])); playbackLength &= FT800EMU_ADDR_MASK;
	uint16_t playbackFreq = *static_cast<uint16_t *>(static_cast<void *>(&ram[REG_PLAYBACK_FREQ]));
	double secondsPerPlaybackSample = 1.0 / (double)playbackFreq;
	uint8_t &playbackFormat = ram[REG_PLAYBACK_FORMAT];
	uint8_t &playbackLoop = ram[REG_PLAYBACK_LOOP];
	uint8_t &playbackBusy = ram[REG_PLAYBACK_PLAY];
	uint32_t &playbackReadPtr = *static_cast<uint32_t *>(static_cast<void *>(&ram[REG_PLAYBACK_READPTR])); playbackReadPtr &= FT800EMU_ADDR_MASK;
	uint8_t &playbackVolume = ram[REG_VOL_PB];
	bool adpcmNext = s_ADPCMNext;

	int audioFrequency = s_AudioOutput->getFrequency();
	double secondsPerSample = 1.0 / (double)audioFrequency;
	int channels = s_AudioOutput->getChannels();

	for (int i = 0; i < samples; ++i)
	{
		int16_t synth;
		if (audioFrequency != FT800EMU_SYNTH_RATE)
		{
			while (s_SecondsPassedForSynthSample > s_SecondsPerSynthSample)
			{
				s_SecondsPassedForSynthSample -= s_SecondsPerSynthSample;
				synthSample0 = synthSample1;
				synthSample1 = AudioProcessor.execute(busy, sound, volume);
			}
			s_SecondsPassedForSynthSample += secondsPerSample;
			synth = (int16_t)(
				(
					((double)synthSample0 * (s_SecondsPerSynthSample - s_SecondsPassedForSynthSample))
					+ ((double)synthSample1 * s_SecondsPassedForSynthSample)
				)
				/ s_SecondsPerSynthSample);
		}
		else
		{
			synth = AudioProcessor.execute(busy, sound, volume);
		}

		int16_t pb;
		if (audioFrequency != playbackFreq && playbackFreq >= 8000 && playbackFreq <= 48000)
		{
			while (s_SecondsPassedForPlaybackSample > secondsPerPlaybackSample)
			{
				s_SecondsPassedForPlaybackSample -= secondsPerPlaybackSample;
				playbackSample0 = playbackSample1;
				playbackSample1 = playback(playbackStart, playbackLength, playbackFormat, playbackLoop, playbackBusy, playbackReadPtr, playbackVolume, adpcmNext, ram);
			}
			s_SecondsPassedForPlaybackSample += secondsPerSample;
			pb = (int16_t)(
				(
					((double)playbackSample0 * (secondsPerPlaybackSample - s_SecondsPassedForPlaybackSample))
					+ ((double)playbackSample1 * s_SecondsPassedForPlaybackSample)
				)
				/ secondsPerPlaybackSample);
		}
		else
		{
			pb = playback(playbackStart, playbackLength, playbackFormat, playbackLoop, playbackBusy, playbackReadPtr, playbackVolume, adpcmNext, ram);
		}

		uint16_t sample = synth + pb;
		for (int j = 0; j < channels && j < 2; ++j)
		{
			audioBuffer[(i * channels) + j] = sample;
		}
		for (int j = 2; j < channels; ++j)
		{
			audioBuffer[(i * channels) + j] = 0;
		}
	}
	s_SynthSample0 = synthSample0;
	s_SynthSample1 = synthSample1;
	s_PlaybackSample0 = playbackSample0;
	s_PlaybackSample1 = playbackSample1;
	s_ADPCMNext = adpcmNext;
}

} /* namespace FT800EMU */

/* end of file */
