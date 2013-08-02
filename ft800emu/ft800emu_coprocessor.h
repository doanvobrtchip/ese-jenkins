/**
 * J1 coprocessor
 * $Id$
 * \file ft800emu_coprocessor.h
 * \brief J1 coprocessor
 * \date 2013-08-03 02:10GMT
 */

#ifndef FT800EMU_COPROCESSOR_H
#define FT800EMU_COPROCESSOR_H
// #include <...>

// System includes

// Project includes

namespace FT800EMU {

/**
 * CoprocessorClass
 * \brief CoprocessorClass
 * \date 2013-08-03 02:10GMT
 */
class CoprocessorClass
{
public:
	CoprocessorClass() { }

	static void begin();
	static void end();

	static void execute();

private:
	CoprocessorClass(const CoprocessorClass &);
	CoprocessorClass &operator=(const CoprocessorClass &);
	
}; /* class CoprocessorClass */

extern CoprocessorClass Coprocessor;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_COPROCESSOR_H */

/* end of file */
