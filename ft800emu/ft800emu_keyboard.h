/**
 * KeyboardClass
 * $Id$
 * \file ft800emu_keyboard.h
 * \brief KeyboardClass
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_KEYBOARD_H
#define FT800EMU_KEYBOARD_H
// #include <...>

// System includes

// Project includes

namespace FT800EMU {

/**
 * KeyboardClass
 * Used for writing arrows to input pins.
 * \brief KeyboardClass
 * \date 2011-05-29 22:02GMT
 * \author Jan Boon (Kaetemi)
 */
class KeyboardClass
{
public:
	KeyboardClass() { }

	static void begin();
	static void update();
	static void end();

	static bool isKeyDown(int key);
	
private:
	KeyboardClass(const KeyboardClass &);
	KeyboardClass &operator=(const KeyboardClass &);
	
}; /* class KeyboardClass */

extern KeyboardClass Keyboard;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_KEYBOARD_H */

/* end of file */
