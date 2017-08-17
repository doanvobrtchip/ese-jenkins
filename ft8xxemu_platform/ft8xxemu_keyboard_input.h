/**
 * KeyboardInput
 * $Id$
 * \file ft8xxemu_keyboard_input.h
 * \brief KeyboardInput
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT8XXEMU_KEYBOARD_INPUT_H
#define FT8XXEMU_KEYBOARD_INPUT_H
// #include <...>

// System includes

// Project includes
#include "ft8xxemu_system.h"

namespace FT8XXEMU {
	class WindowOutput;

/**
 * KeyboardInput
 * \brief KeyboardInput
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */
class KeyboardInput
{
public:
	static KeyboardInput *create(System *system, WindowOutput *windowOutput);
	void destroy();

private:
	KeyboardInput(System *system, WindowOutput *windowOutput);
	virtual ~KeyboardInput();

public:
	void update();
	bool isKeyDown(int key);

private:
	void release();

private:
	System *m_System;
	WindowOutput *m_WindowOutput;
	LPDIRECTINPUT8 m_lpDI = NULL;
	LPDIRECTINPUTDEVICE8 m_lpDIKeyboard = NULL;
	char m_BufferKeyboard[256] = { 0 };
	
private:
	KeyboardInput(const KeyboardInput &) = delete;
	KeyboardInput &operator=(const KeyboardInput &) = delete;
	
}; /* class KeyboardInput */

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_KEYBOARD_INPUT_H */

/* end of file */
