/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FT800EMU_BUS_SLAVE_H
#define FT800EMU_BUS_SLAVE_H
#include "bt8xxemu_inttypes.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

// System includes
#include <stdlib.h>

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Memory;

/**
 * BusSlave
 * \brief BusSlave
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */
class BusSlave
{
public:
	BusSlave(FT8XXEMU::System *system, Memory *memory);
	~BusSlave();

	void cs(bool cs);
	uint8_t transfer(uint8_t data);

private:
	enum BusSlaveState
	{
		BusSlaveIdle,
		BusSlaveNotImplemented,
		BusSlaveInvalidState,
		BusSlaveReadAddress,
		BusSlaveWriteAddress,
		BusSlaveRead,
		BusSlaveWrite,
	};

private:
	bool m_CS = false;

	uint32_t m_RWBuffer = 0;
	uint32_t m_RWBufferStage = 0;

	uint32_t m_WriteStartAddr = 0;

	FT8XXEMU::System *m_System;
	Memory *m_Memory;

	BusSlaveState m_State = BusSlaveIdle;
	uint32_t m_Cursor = 0;
	int m_Stage = 0;

private:
	BusSlave(const BusSlave &) = delete;
	BusSlave &operator=(const BusSlave &) = delete;

}; /* class BusSlave */

} /* namespace FT800EMU */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* #ifndef FT800EMU_BUS_SLAVE_H */

/* end of file */
