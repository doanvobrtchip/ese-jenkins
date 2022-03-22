/*
BT817 Emulator Library
Copyright (C) 2017-2020  Bridgetek Pte Lte
Author: James Bowman <jamesb@excamera.com>
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef BT817EMU_ESPIM_H
#define BT817EMU_ESPIM_H

#include "bt8xxemu.h"

namespace BT817EMU {
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

} /* namespace BT817EMU */

#endif /* #ifndef BT817EMU_ESPIM_H */

/* end of file */
