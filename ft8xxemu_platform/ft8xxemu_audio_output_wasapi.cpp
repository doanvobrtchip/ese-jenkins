/**
 * AudioOutput
 * $Id$
 * \file ft8xxemu_audio_output_wasapi.cpp
 * \brief AudioOutput
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef WIN32

// #include <...>
#include "ft8xxemu_audio_output.h"

// System includes
#include "ft8xxemu_system_windows.h"

// Project includes

// using namespace ...;

namespace FT8XXEMU {

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define D_BT8XXEMU_AUDIOCHANNELS 2
#define D_BT8XXEMU_AUDIOBITS 16

AudioOutput *AudioOutput::create()
{
	return new AudioOutput();
}

void AudioOutput::destroy()
{
	delete this;
}

AudioOutput::AudioOutput()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	HRESULT hr;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&m_MMDeviceEnumerator);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	hr = m_MMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_MMDevice);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	hr = m_MMDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&m_AudioClient);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	WAVEFORMATEX *pwfx;
	hr = m_AudioClient->GetMixFormat(&pwfx);
	m_AudioFrequency = pwfx->nSamplesPerSec;

	WAVEFORMATEX wfx;
	wfx.wFormatTag = 0x0001;
	wfx.nChannels = D_BT8XXEMU_AUDIOCHANNELS;
	wfx.nSamplesPerSec = pwfx->nSamplesPerSec;
	wfx.nBlockAlign = D_BT8XXEMU_AUDIOCHANNELS * D_BT8XXEMU_AUDIOBITS / 8;
	wfx.nAvgBytesPerSec = pwfx->nSamplesPerSec * wfx.nBlockAlign;
	wfx.wBitsPerSample = D_BT8XXEMU_AUDIOBITS;
	wfx.cbSize = 0;

	long hnsRequestedDuration = REFTIMES_PER_SEC / 10;
	hr = m_AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, &wfx, NULL);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	hr = m_AudioClient->GetBufferSize(&m_BufferFrameCount);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	hr = m_AudioClient->GetService(IID_IAudioRenderClient, (void **)&m_AudioRenderClient);
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }

	hr = m_AudioClient->Start();
	if (hr) { SystemWindows.WarningHResult(TEXT("WASAPI Initialisation"), hr); }
}

AudioOutput::~AudioOutput()
{

}

int AudioOutput::getFrequency()
{
	return m_AudioFrequency;
}

int AudioOutput::getChannels()
{
	return 2;
}

void AudioOutput::beginBuffer(short **buffer, int *samples)
{
	HRESULT hr;

	unsigned int numFramesPadding;

	hr = m_AudioClient->GetCurrentPadding(&numFramesPadding);
	if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);

	m_NumFramesAvailable = m_BufferFrameCount - numFramesPadding;
	*samples = m_NumFramesAvailable;

	if (m_NumFramesAvailable == 0)
	{
		buffer = NULL;
	}
	else
	{
		hr = m_AudioRenderClient->GetBuffer(m_NumFramesAvailable, (BYTE **)buffer);
		if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);
	}
}

void AudioOutput::endBuffer()
{
	HRESULT hr;

	hr = m_AudioRenderClient->ReleaseBuffer(m_NumFramesAvailable, 0);
	if (hr) SystemWindows.ErrorHResult(TEXT("WASAPI Buffer"), hr);
}

} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */

/* end of file */
