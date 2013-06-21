/**
 * KeyboardClass
 * $Id$
 * \file ft800emu_keyboard_directinput.cpp
 * \brief KeyboardClass
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_SDL

// #include <...>
#include "ft800emu_keyboard.h"

// System includes

// Project includes
#include "ft800emu_system_windows.h"
#include "ft800emu_system.h"
#include "wiring.h"

// using namespace ...;

#define FT800EMU_DIGITAL_LEFT  6
#define FT800EMU_DIGITAL_RIGHT 3
#define FT800EMU_DIGITAL_UP    4
#define FT800EMU_DIGITAL_DOWN  5
#define FT800EMU_DIGITAL_SHOOT 2
#define FT800EMU_ANALOG_X      0
#define FT800EMU_ANALOG_Y      1

namespace FT800EMU {

KeyboardClass Keyboard;

static LPDIRECTINPUT8 s_lpDI = NULL;
static LPDIRECTINPUTDEVICE8 s_lpDIKeyboard = NULL;
static unsigned char s_BufferKeyboard[256];

void KeyboardClass::begin()
{
	HRESULT hr;

	if (s_lpDI) SystemWindows.Error(TEXT("KeyboardClass::begin()  s_lpDI != NULL"));
	hr = DirectInput8Create((HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_lpDI, NULL);
	if FAILED(hr) { end(); SystemWindows.Error(TEXT("DirectInput not available")); }

	// Retrieve interface to an IDirectInputDevice8
	if (s_lpDIKeyboard) SystemWindows.Error(TEXT("KeyboardClass::begin()  m_lpDIKeyboard != NULL"));
    hr = s_lpDI->CreateDevice(GUID_SysKeyboard, &s_lpDIKeyboard, NULL);
	if FAILED(hr) { end(); SystemWindows.Error(TEXT("Keyboard not available (1)")); }

	// Set keyboard data format
	hr = s_lpDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if FAILED(hr) { end(); SystemWindows.Error(TEXT("Keyboard not available (2)")); }

	// Set cooperative level
	hr = s_lpDIKeyboard->SetCooperativeLevel(SystemWindows.getHWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if FAILED(hr) { end(); SystemWindows.Error(TEXT("Keyboard not available (3)")); }

	// Get access to the input device, may fail the first time round
    hr = s_lpDIKeyboard->Acquire();
    //if FAILED(hr) { Input::Release(); Utilities::Error(TEXT("Keyboard not available (4)")); } // doesn't matter
}

void KeyboardClass::update()
{
	HRESULT hr;

    hr = s_lpDIKeyboard->GetDeviceState(sizeof(s_BufferKeyboard),(LPVOID)&s_BufferKeyboard);
	if FAILED(hr)
	{
		// Reacquire access to the input device
		memset(s_BufferKeyboard, 0, sizeof(s_BufferKeyboard));
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
		{
			s_lpDIKeyboard->Acquire();
		}
		else
		{
			end();
			SystemWindows.Error(TEXT("Keyboard not available (5)"));
			//return false;
		}
	}

	digitalWrite(FT800EMU_DIGITAL_LEFT, ((s_BufferKeyboard[DIK_LEFT] & 0x80) != 0) ? LOW : HIGH);
	digitalWrite(FT800EMU_DIGITAL_RIGHT, ((s_BufferKeyboard[DIK_RIGHT] & 0x80) != 0) ? LOW : HIGH);
	digitalWrite(FT800EMU_DIGITAL_UP, ((s_BufferKeyboard[DIK_UP] & 0x80) != 0) ? LOW : HIGH);
	digitalWrite(FT800EMU_DIGITAL_DOWN, ((s_BufferKeyboard[DIK_DOWN] & 0x80) != 0) ? LOW : HIGH);
	digitalWrite(FT800EMU_DIGITAL_SHOOT, ((s_BufferKeyboard[DIK_SPACE] & 0x80) != 0) ? LOW : HIGH);

	uint16_t analogX = 511;
	uint16_t analogY = 511;
	if (((s_BufferKeyboard[DIK_Q] & 0x80) != 0) || ((s_BufferKeyboard[DIK_A] & 0x80) != 0))
		analogX -= 511;
	if (((s_BufferKeyboard[DIK_D] & 0x80) != 0))
		analogX += 511;
	if (((s_BufferKeyboard[DIK_S] & 0x80) != 0))
		analogY -= 511;
	if (((s_BufferKeyboard[DIK_Z] & 0x80) != 0) || ((s_BufferKeyboard[DIK_W] & 0x80) != 0))
		analogY += 511;

	System.setAnalogRead(FT800EMU_ANALOG_X, analogX);
	System.setAnalogRead(FT800EMU_ANALOG_Y, analogY);

	//return true;
	return;
}

void KeyboardClass::end()
{
	if (s_lpDIKeyboard)
	{
		s_lpDIKeyboard->Unacquire();
		s_lpDIKeyboard->Release();
		s_lpDIKeyboard = NULL;
	}
	else SystemWindows.Debug(TEXT("KeyboardClass::end()  s_lpDIKeyboard == NULL"));
	if (s_lpDI)
	{
		s_lpDI->Release();
		s_lpDI = NULL;
	}
	else SystemWindows.Debug(TEXT("KeyboardClass::end()  s_lpDI == NULL"));
}

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_SDL */

/* end of file */
