/*
BT8XX Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

// #include <...>
#include "ft8xxemu_thread_state.h"

// System includes
#include "ft8xxemu_system_win32.h"

// Project includes

// using namespace ...;

namespace FT8XXEMU {

ThreadState::ThreadState()
{
	
}

ThreadState::~ThreadState()
{
	reset();
}

} /* namespace FT8XXEMU */

/* end of file */
