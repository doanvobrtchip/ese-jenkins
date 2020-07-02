/*
BT815 Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Author: Jan Boon <jan@no-break.space>
*/

#ifndef BT815EMU_ESPIM_H
#define BT815EMU_ESPIM_H

#include "bt8xxemu.h"

namespace BT815EMU {
	class Memory;

#pragma warning(push)
#pragma warning(disable : 26495)
class Espim
{
private:
	uint32_t count, a;
	int state;
	uint8_t d[64];
	Memory *m_Memory;

public:
	Espim(Memory *memory);
	int running();
	void drive(); // uint32_t spimi, uint32_t &spimo, uint32_t &spim_dir, uint32_t &spim_clken);
	void trigger();

};
#pragma warning(pop)

} /* namespace FT800EMU */

#endif /* #ifndef BT815EMU_ESPIM_H */

/* end of file */
