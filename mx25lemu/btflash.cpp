/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include "bt8xxemu.h"
#include "bt8xxemu_flash.h"

#include "btflash_defs.h"

#include <stdio.h>
#include <stdarg.h>

#include <mutex>

#define Flash_debug(message, ...) log(BT8XXEMU_LogMessage, "[[DEBUG]] " message, __VA_ARGS__)
// #define Flash_debug(message, ...)

extern BT8XXEMU_FlashVTable g_FlashVTable;
static std::mutex s_LogMutex;

class Flash : public BT8XXEMU::Flash
{
public:
	Flash(const BT8XXEMU_FlashParameters *params) : BT8XXEMU::Flash(&g_FlashVTable),
		m_DeepPowerDown(false), m_ChipSelect(false), m_TransferCmd(0)
	{
		static_assert(offsetof(Flash, m_VTable) == 0, "Incompatible C++ ABI");

		m_Log = params->Log;
		m_UserContext = params->UserContext;
		m_PrintStd = true;

		m_StatusRegister = 0;
	}

	~Flash()
	{

	}

	void chipSelect(bool cs)
	{
		m_ChipSelect = cs;
		m_TransferCmd = 0;
		m_TransferNb = -1;
	}

	void writeProtect(bool wp)
	{

	}

	void hold(bool hold)
	{

	}

	uint8_t transfer(uint8_t data)
	{
		if (m_ChipSelect)
		{
			if (m_DeepPowerDown)
			{
				if (data == BTFLASH_CMD_RDP) /* Release from Deep Power Down ¨*/
				{
					m_DeepPowerDown = false;
					m_TransferCmd = BTFLASH_CMD_DP;
				}
				return 0;
			}
			++m_TransferNb;
			switch (m_TransferCmd)
			{
			case 0:
				m_TransferCmd = data;
				switch (data)
				{
				case BTFLASH_CMD_WREN: /* Write Enable */
					Flash_debug("Write Enable");
					m_StatusRegister |= BTFLASH_STATUS_WEL_FLAG;
					return 0;
				case BTFLASH_CMD_WRDI: /* Write Disable */
					Flash_debug("Write Disable");
					m_StatusRegister &= ~BTFLASH_STATUS_WEL_FLAG;
					return 0;
				case BTFLASH_CMD_RDID: /* Read Identification */
					Flash_debug("Read Identification");
					return 0xC2;
				case BTFLASH_CMD_RDSR: /* Read Status Register */
					Flash_debug("Read Status Register");
					return m_StatusRegister;
				case BTFLASH_CMD_WRSR: /* Write Status Register */
				case BTFLASH_CMD_READ: /* Read Data */
				case BTFLASH_CMD_FAST_READ: /* Fast Read Data */
				case BTFLASH_CMD_2READ: /* 2x IO Read */
				case BTFLASH_CMD_SE: /* Sector Erase */
				case BTFLASH_CMD_BE: /* Block Erase */
				case BTFLASH_CMD_CE_60: /* Chip Erase */
				case BTFLASH_CMD_CE_C7: /* Chip Erase */
				case BTFLASH_CMD_PP: /* Page Program */
				case BTFLASH_CMD_CP: /* Continuously Program mode */
				case BTFLASH_CMD_DP: /* Deep Power Down */
				case BTFLASH_CMD_RES: /* Read Electronic ID */
				case BTFLASH_CMD_REMS: /* Read Electronic Manufacturer and Device ID */
				case BTFLASH_CMD_REMS2: /* Read ID for 2x IO Mode */
				case BTFLASH_CMD_ENSO: /* Enter Secured OTP */
				case BTFLASH_CMD_EXSO: /* Exit Secured OTP */
				case BTFLASH_CMD_RDSCUR: /* Read Security Register */
				case BTFLASH_CMD_WRSCUR: /* Write Security Register */
				case BTFLASH_CMD_ESRY: /* Enable SO to output RY/BY# */
				case BTFLASH_CMD_DSRY: /* Disable SO to output RY/BY# */
					log(BT8XXEMU_LogError, "Flash command not implemented (%x)", data);
					break;
				default:
					log(BT8XXEMU_LogError, "Flash command unrecognized (%x)", data);
					break;
				}
				break;
			case BTFLASH_CMD_RDID:
				switch (m_TransferNb)
				{
				case 1:
					return 0x20;
				case 2:
					if (Size >= 8 * 1024 * 2014) return 0x17;
					else if (Size >= 4 * 1024 * 2014) return 0x16;
					else if (Size >= 2 * 1024 * 2014) return 0x15;
					else
					{
						log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
						break;
					}
				default:
					log(BT8XXEMU_LogWarning, "Read Identification exceeded transfer length");
					break;
				}
				break;
			default:
				log(BT8XXEMU_LogError, "Flash command (%x) exceeded transfer length", m_TransferCmd);
				break;
			}
		}
		return 0;
	}

	uint8_t *Data;
	uint32_t Size;

private:
	bool m_DeepPowerDown;

	bool m_ChipSelect;
	uint8_t m_TransferCmd;
	int m_TransferNb;

	void(*m_Log)(BT8XXEMU_Flash *sender, void *context, BT8XXEMU_LogType type, const char *message);
	void *m_UserContext;
	bool m_PrintStd;

	uint8_t m_StatusRegister;

private:
	void log(BT8XXEMU_LogType type, const char *message, ...)
	{
		char buffer[2048];
		va_list args;
		va_start(args, message);
		vsnprintf(buffer, 2047, message, args);
		; {
			std::unique_lock<std::mutex> lock(s_LogMutex);
			m_Log && (m_Log(this, m_UserContext, type, buffer), true);
			const char *level =
				(type == BT8XXEMU_LogMessage
					? "Message"
					: (type == BT8XXEMU_LogWarning
						? "Warning" : "Error"));
			m_PrintStd && (printf("[%s] %s\n", level, buffer));
		}
		va_end(args);
	}

};

void Flash_destroy(Flash *flash)
{
	delete flash;
}

void Flash_chipSelect(Flash *flash, bool cs)
{
	flash->chipSelect(cs);
}

void Flash_writeProtect(Flash *flash, bool wp)
{
	flash->writeProtect(wp);
}

void Flash_hold(Flash *flash, bool hold)
{
	flash->hold(hold);
}

uint8_t Flash_transfer(Flash *flash, uint8_t data)
{
	return flash->transfer(data);
}

uint8_t *Flash_data(Flash *flash)
{
	return flash->Data;
}

uint32_t Flash_size(Flash *flash)
{
	return flash->Size;
}

BT8XXEMU_FlashVTable g_FlashVTable = {
	(void(*)(BT8XXEMU::Flash *))Flash_destroy,

	(void(*)(BT8XXEMU::Flash *, bool))Flash_chipSelect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_writeProtect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_hold,
	(uint8_t(*)(BT8XXEMU::Flash *, uint8_t))Flash_transfer,

	(uint8_t *(*)(BT8XXEMU::Flash *))Flash_data,
	(uint32_t(*)(BT8XXEMU::Flash *))Flash_size
};

BT8XXEMU_EXPORT BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return NULL;
	}

	return new Flash(params);
}

/* end of file */
