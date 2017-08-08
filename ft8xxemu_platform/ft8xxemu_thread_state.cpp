/**
 * ThreadState
 * \file ft8xxemu_thread_state.cpp
 * \brief ThreadState
 * \date 2017-08-04 10:57GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

// #include <...>
#include "ft8xxemu_thread_state.h"

// System includes
#include "ft8xxemu_system_windows.h"

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
