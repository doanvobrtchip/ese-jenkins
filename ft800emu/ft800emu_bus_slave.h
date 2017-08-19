/**
 * BusSlave
 * $Id$
 * \file ft800emu_bus_slave.h
 * \brief BusSlave
 * \date 2013-06-21 21:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013-2017  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_BUS_SLAVE_H
#define FT800EMU_BUS_SLAVE_H
#include "ft8xxemu_inttypes.h"

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

#endif /* #ifndef FT800EMU_BUS_SLAVE_H */

/* end of file */
