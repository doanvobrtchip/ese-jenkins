/**
 * AudioDriverClass
 * $Id$
 * \file ft800emu_audio_driver_sdl.cpp
 * \brief AudioDriverClass
 * \date 2012-06-27 11:45GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef FT800EMU_SDL

// #include <...>
#include "ft800emu_audio_driver.h"

// System includes
#include "ft800emu_system_sdl.h"

// Project includes
#include "ft800emu_audio_render.h"

// using namespace ...;

namespace FT800EMU {

AudioDriverClass AudioDriver;

static int s_AudioFrequency = 0;
static int s_AudioChannels = 0;

namespace {

void sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	AudioRender.process((short *)stream, len / 2); // 1 channel, 2 bytes per channel (hopefully...)
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
