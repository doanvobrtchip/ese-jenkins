/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#if 0

#if (defined(FTEMU_SDL) || defined(FTEMU_SDL2))

// #include <...>
#include "ft8xxemu_audio_output.h"

// System includes
#include "ft8xxemu_system_sdl.h"

// using namespace ...;

namespace FT8XXEMU {

AudioOutput AudioOutput;

void (*g_AudioProcess)(short *audioBuffer, int samples) = NULL;

static int s_AudioFrequency = 0;
static int s_AudioChannels = 0;

namespace {

void sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	g_AudioProcess((short *)stream, len / 2); // 1 channel, 2 bytes per channel (hopefully...)
}

} /* anonymous namespace */

bool AudioOutput::begin()
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
	{
		SystemSdl.IgnoreErrorSdl();
	}

	s_AudioFrequency = obtained.freq;
	s_AudioChannels = obtained.channels;

	if (s_AudioChannels != 1)
	{
		printf("Bad nb channels\n");
		exit(EXIT_FAILURE);
	}

	SDL_PauseAudio(0);
}

bool AudioOutput::update()
{
	return true;
}

int AudioOutput::getFrequency()
{
	return s_AudioFrequency;
}

int AudioOutput::getChannels()
{
	return s_AudioChannels;
}

void AudioOutput::beginBuffer(short **buffer, int *samples)
{
	*buffer = NULL;
	*samples = 0;
}

void AudioOutput::endBuffer()
{

}

void AudioOutput::end()
{
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

} /* namespace FT8XXEMU */

#endif /* #ifdef FTEMU_SDL */

#endif

/* end of file */
