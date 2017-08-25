/**
 * AudioOutput
 * $Id$
 * \file ft8xxemu_audio_output.h
 * \brief AudioOutput
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifndef BT8XXEMU_AUDIO_OUTPUT_H
#define BT8XXEMU_AUDIO_OUTPUT_H
// #include <...>

// Project includes
#include "ft8xxemu_system.h"

// System includes
#include <functional>

// WASAPI
#include <objbase.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

namespace FT8XXEMU {

/**
 * AudioOutput
 * \brief AudioOutput
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */
class AudioOutput
{
public:
	static AudioOutput *create(System *system);
	void destroy();

private:
	AudioOutput(System *system);
	virtual ~AudioOutput();

public:
	void onAudioProcess(std::function<void(short *audioBuffer, int samples)> audioProcess) { m_AudioProcess = audioProcess; }; // replaces g_AudioProcess
	// bool update() = 0;

	int getFrequency(); // WASAPI doesn't resample
	int getChannels(); // WASAPI doesn't adjust channels

	void beginBuffer(short **buffer, int *samples);
	void endBuffer();

private:
	std::function<void(short *audioBuffer, int samples)> m_AudioProcess; // TODO: 2017-08-09: SDL2: Empty default handler to blank audioBuffer

	System *m_System;

#ifdef WIN32
	IMMDeviceEnumerator *m_MMDeviceEnumerator = NULL;
	IMMDevice *m_MMDevice = NULL;
	IAudioClient *m_AudioClient = NULL;
	IAudioRenderClient *m_AudioRenderClient = NULL;

	int m_AudioFrequency = 0;
	unsigned int m_BufferFrameCount = 0;
	unsigned int m_NumFramesAvailable = 0;
#endif

private:
	AudioOutput(const AudioOutput &) = delete;
	AudioOutput &operator=(const AudioOutput &) = delete;

}; /* class AudioOutput */

} /* namespace FT8XXEMU */

#endif /* #ifndef BT8XXEMU_AUDIO_OUTPUT_H */

/* end of file */
