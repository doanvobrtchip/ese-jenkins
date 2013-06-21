/**
 * AudioDriverClass
 * $Id$
 * \file ft800emu_audio_driver_sdl.cpp
 * \brief AudioDriverClass
 * \date 2012-06-27 11:45GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011-2012  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL

// #include <...>
#include "ft800emu_audio_driver.h"

// System includes
#include "ft800emu_system_sdl.h"

// Project includes
#include "ft800emu_audio_machine.h"

// using namespace ...;

namespace FT800EMU {

AudioDriverClass AudioDriver;
/*
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

static IMMDeviceEnumerator *s_MMDeviceEnumerator = NULL;
static IMMDevice *s_MMDevice = NULL;
static IAudioClient *s_AudioClient = NULL;
static IAudioRenderClient *s_AudioRenderClient = NULL;

static unsigned int s_BufferFrameCount = 0;
static unsigned int s_NumFramesAvailable = 0;

static double s_TestRad;
*/

static int s_AudioFrequency = 0;

/*
#define D_FT800EMU_AUDIOCHANNELS 2
#define D_FT800EMU_AUDIOBITS 16*/

namespace {

void sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	AudioMachine.process((short *)stream, len / 4); // 2 channels, 2 bytes per channel (hopefully...)
}

} /* anonymous namespace */

void AudioDriverClass::begin()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	desired.freq = 22050;
	desired.format = AUDIO_S16LSB;
	desired.channels = 2;
	desired.samples = 512; // 2048; //8192;
	desired.callback = sdlAudioCallback;
	desired.userdata = NULL;

	if (SDL_OpenAudio(&desired, &obtained) < 0)
		SystemSdl.ErrorSdl();

	s_AudioFrequency = obtained.freq;

	SDL_PauseAudio(0);
}

bool AudioDriverClass::update()
{
	return true;
}

int AudioDriverClass::getFrequency()
{
	return s_AudioFrequency;
}

void AudioDriverClass::beginBuffer(short **buffer, int *samples)
{
	buffer = NULL;
	samples = 0;
}

void AudioDriverClass::endBuffer()
{

}

void AudioDriverClass::end()
{
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

} /* namespace FT800EMU */

#endif /* #ifdef FT800EMU_SDL */

/* end of file */
