/**
 * J1 coprocessor
 * $Id$
 * \file ft800emu_coprocessor.cpp
 * \brief J1 coprocessor
 * \date 2013-08-03 02:10GMT
*/

// #include <...>
#include "ft800emu_coprocessor.h"

// System includes

// Project includes
#include "ft800emu_memory.h"
#include "ft800emu_system.h"
#include "vc.h"

// using namespace ...;

namespace FT800EMU {

CoprocessorClass Coprocessor;

void CoprocessorClass::begin()
{
	
}

void CoprocessorClass::execute()
{
	// loop until end exec
	while (true)
	{
		// dummy
		System.delay(100);
	}
}

void CoprocessorClass::end()
{

}

} /* namespace GDEMU */

/* end of file */
