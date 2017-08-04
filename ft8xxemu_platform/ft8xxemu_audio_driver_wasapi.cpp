/**
 * AudioDriverClass
 * $Id$
 * \file ft8xxemu_audio_driver_wasapi.cpp
 * \brief AudioDriverClass
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEMU_SDL
#ifndef FTEMU_SDL2

// #include <...>
#include "ft8xxemu_audio_driver.h"

// System includes
#include "ft8xxemu_system_windows.h"

// Project includes

// using namespace ...;

namespace FT8XXEMU {

AudioDriverClass AudioDriver;

void (*g_AudioProcess)(short *audioBuffer, int samples) = NULL;

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

static IMMDeviceEnumerator *s_MMDeviceEnumerator = NULL;
static IMMDevice *s_MMDevice = NULL;
static IAudioClient *s_AudioClient = NULL;
static IAudioRenderClient *s_AudioRenderClient = NULL;

static int s_AudioFrequency = 0;
static unsigned int s_BufferFrameCount = 0;
static unsigned int s_NumFramesAvailable = 0;

static double s_TestRad;

#define D_BT8XXEMU_AUDIOCHANNELS 2
#define D_BT8XXEMU_AUDIOBITS 16

bool AudioDriverClass::begin()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	HRESULT hr;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&s_MMDeviceEnumerator);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	hr = s_MMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &s_MMDevice);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	hr = s_MMDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&s_AudioClient);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	WAVEFORMATEX *pwfx;
	hr = s_AudioClient->GetMixFormat(&pwfx);
	s_AudioFrequency = pwfx->nSamplesPerSec;

	WAVEFORMATEX wfx;
	wfx.wFormatTag = 0x0001;
	wfx.nChannels = D_BT8XXEMU_AUDIOCHANNELS;
	wfx.nSamplesPerSec = pwfx->nSamplesPerSec;
	wfx.nBlockAlign = D_BT8XXEMU_AUDIOCHANNELS * D_BT8XXEMU_AUDIOBITS / 8;
	wfx.nAvgBytesPerSec = pwfx->nSamplesPerSec * wfx.nBlockAlign;
	wfx.wBitsPerSample = D_BT8XXEMU_AUDIOBITS;
	wfx.cbSize = 0;

	long hnsRequestedDuration = REFTIMES_PER_SEC / 10;
	hr = s_AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, &wfx, NULL);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	hr = s_AudioClient->GetBufferSize(&s_BufferFrameCount);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	hr = s_AudioClient->GetService(IID_IAudioRenderClient, (void **)&s_AudioRenderClient);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	hr = s_AudioClient->Start();
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); return false; }

	// done
	return true;
}

bool AudioDriverClass::update()
{
	// sin test

	/*short *audioData;

	int hr;
	unsigned int numFramesAvailable;
	unsigned int numFramesPadding;

	// test
	hr = s_AudioClient->GetCurrentPadding(&numFramesPadding);
	if (hr) SystemWindows.ErrorHResult(hr);

	numFramesAvailable = s_BufferFrameCount - numFramesPadding;
	hr = s_AudioRenderClient->GetBuffer(numFramesAvailable, (BYTE **)&audioData);
	if (hr) SystemWindows.ErrorHResult(hr);

	double addRad = 3.14159 / (double)s_AudioFrequency * 2.0 * 440.0;
	// fill
	for (unsigned int i = 0; i < numFramesAvailable; ++i)
	{
		s_TestRad += addRad;
		audioData[i * 2] = (short)((sin(s_TestRad)) * 5000);
		audioData[(i * 2) + 1] = audioData[i * 2];
	}

	s_AudioRenderClient->ReleaseBuffer(numFramesAvailable, 0);
	if (hr) SystemWindows.ErrorHResult(hr);*/

	return true;
}

int AudioDriverClass::getFrequency()
{
	return s_AudioFrequency;
}

int AudioDriverClass::getChannels()
{
	return 2;
}

void AudioDriverClass::beginBuffer(short **buffer, int *samples)
{
	HRESULT hr;

	unsigned int numFramesPadding;

	hr = s_AudioClient->GetCurrentPadding(&numFramesPadding);
	if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);

	s_NumFramesAvailable = s_BufferFrameCount - numFramesPadding;
	*samples = s_NumFramesAvailable;

	if (s_NumFramesAvailable == 0)
	{
		buffer = NULL;
	}
	else
	{
		hr = s_AudioRenderClient->GetBuffer(s_NumFramesAvailable, (BYTE **)buffer);
		if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);
	}
}

void AudioDriverClass::endBuffer()
{
	HRESULT hr;

	hr = s_AudioRenderClient->ReleaseBuffer(s_NumFramesAvailable, 0);
	if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);
}

void AudioDriverClass::end()
{
	// todo*/***
}

} /* namespace FT8XXEMU */

#endif /* #ifndef FTEMU_SDL2 */
#endif /* #ifndef FTEMU_SDL */

/* end of file */
