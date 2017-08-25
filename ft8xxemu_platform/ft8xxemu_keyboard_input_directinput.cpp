/**
 * KeyboardInput
 * $Id$
 * \file ft8xxemu_keyboard_directinput.cpp
 * \brief KeyboardInput
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifdef WIN32

#include "ft8xxemu_keyboard_input.h"

// System includes
#include "ft8xxemu_system_win32.h"
#include "ft8xxemu_system.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Project includes
#include "ft8xxemu_window_output.h"

// using namespace ...;

namespace FT8XXEMU {

KeyboardInput *KeyboardInput::create(System *system, WindowOutput *windowOutput)
{
	return new KeyboardInput(system, windowOutput);
}

void KeyboardInput::destroy()
{
	delete this;
}

KeyboardInput::KeyboardInput(System *system, WindowOutput *windowOutput) : m_System(system), m_WindowOutput(windowOutput)
{
	HRESULT hr;

	if (m_lpDI) FTEMU_error("KeyboardInput::begin()  m_lpDI != NULL");
	hr = DirectInput8Create((HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_lpDI, NULL);
	if FAILED(hr) { release(); FTEMU_error("DirectInput not available"); }

	// Retrieve interface to an IDirectInputDevice8
	if (m_lpDIKeyboard) FTEMU_error("KeyboardInput::begin()  m_lpDIKeyboard != NULL");
    hr = m_lpDI->CreateDevice(GUID_SysKeyboard, &m_lpDIKeyboard, NULL);
	if FAILED(hr) { release(); FTEMU_error("Keyboard not available (1)"); }

	// Set keyboard data format
	hr = m_lpDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if FAILED(hr) { release(); FTEMU_error("Keyboard not available (2)"); }

	// Set cooperative level
	hr = m_lpDIKeyboard->SetCooperativeLevel(windowOutput->getHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if FAILED(hr) { release(); FTEMU_error("Keyboard not available (3)"); }

	// Get access to the input device, may fail the first time round
    hr = m_lpDIKeyboard->Acquire();
    // if FAILED(hr) { Input::Release(); Utilities::Error(TEXT("Keyboard not available (4)")); } // doesn't matter
}

KeyboardInput::~KeyboardInput()
{
	release();
}

void KeyboardInput::update()
{
	HRESULT hr;

    hr = m_lpDIKeyboard->GetDeviceState(sizeof(m_BufferKeyboard), (LPVOID)&m_BufferKeyboard);
	if FAILED(hr)
	{
		// Reacquire access to the input device
		memset(m_BufferKeyboard, 0, sizeof(m_BufferKeyboard));
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
		{
			m_lpDIKeyboard->Acquire();
		}
		else
		{
			release();
			FTEMU_error("Keyboard not available (5)");
		}
	}
}

void KeyboardInput::release()
{
	if (m_lpDIKeyboard)
	{
		m_lpDIKeyboard->Unacquire();
		m_lpDIKeyboard->Release();
		m_lpDIKeyboard = NULL;
	}
	else FTEMU_error("KeyboardInput::end()  m_lpDIKeyboard == NULL");
	if (m_lpDI)
	{
		m_lpDI->Release();
		m_lpDI = NULL;
	}
	else FTEMU_error("KeyboardInput::end()  m_lpDI == NULL");
}

bool KeyboardInput::isKeyDown(int key)
{
	return ((m_BufferKeyboard[key] & 0x80) != 0);
}

} /* namespace FT8XXEMU */

#endif /* #ifdef WIN32 */

/* end of file */
