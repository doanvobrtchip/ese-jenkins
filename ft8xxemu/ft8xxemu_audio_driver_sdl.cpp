/**
 * AudioDriverClass
 * $Id$
 * \file ft8xxemu_audio_driver_sdl.cpp
 * \brief AudioDriverClass
 * \date 2012-06-27 11:45GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

// #include <...>
#include "ft8xxemu_audio_driver.h"

// System includes
#include "ft8xxemu_system_sdl.h"

// using namespace ...;

namespace FT8XXEMU {

AudioDriverClass AudioDriver;

void (*g_AudioProcess)(short *audioBuffer, int samples) = NULL;

static int s_AudioFrequency = 0;
static int s_AudioChannels = 0;

namespace {

void sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	g_AudioProcess((short *)stream, len / 2); // 1 channel, 2 bytes per channel (hopefully...)
}

} /* anonymous namespace */

void AudioDriverClass::begin()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	desired.freq = 48000;
	desired.format = AUDIO_S16LSB;
	desired.channels = 1;
	desired.samples = 512; // 2048; //8192;
	desired.callback = sdlAudioCallback;
	desired.userdata = NULL;

	if (SDL_OpenAudio(&desired, &obtained) < 0)
		SystemSdl.ErrorSdl();

	s_AudioFrequency = obtained.freq;
	s_AudioChannels = obtained.channels;

	if (s_AudioChannels != 1)
	{
		printf("Bad nb channels\n");
		exit(EXIT_FAILURE);
	}

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

int AudioDriverClass::getChannels()
{
	return s_AudioChannels;
}

void AudioDriverClass::beginBuffer(short **buffer, int *samples)
{
	*buffer = NULL;
	*samples = 0;
}

void AudioDriverClass::endBuffer()
{

}

void AudioDriverClass::end()
{
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

} /* namespace FT8XXEMU */

#endif /* #ifdef FTEMU_SDL */

/* end of file */
