/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include "bt8xxemu.h"
#include "bt8xxemu_flash.h"

#include "btflash_defs.h"

#ifdef WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	if !defined(NTDDI_VERSION) && !defined(_WIN32_WINNT) && !defined(WINVER)
#		define NTDDI_VERSION 0x05010000 /* NTDDI_WINXP */
#		define _WIN32_WINNT 0x0501 /* _WIN32_WINNT_WINXP */
#		define WINVER 0x0501 /* _WIN32_WINNT_WINXP */
#	endif
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	include <Shlwapi.h>
#	pragma comment(lib, "Shlwapi.lib")
#endif

#include <stdio.h>
#include <stdarg.h>

#include <algorithm>
#include <mutex>

#define Flash_debug(message, ...) log(BT8XXEMU_LogMessage, "[[DEBUG]] " message, __VA_ARGS__)
// #define Flash_debug(message, ...)

extern BT8XXEMU_FlashVTable g_FlashVTable;
static std::mutex s_LogMutex;
struct AutoUnlockLogMutex
{
	~AutoUnlockLogMutex()
	{
		if (!s_LogMutex.try_lock()) new(&s_LogMutex) std::mutex(); // Force unlock during forced process termination
		else s_LogMutex.unlock();
	}
};
static AutoUnlockLogMutex g_AutoUnlockLogMutex;

int64_t getFileSize(const wchar_t* name);

class Flash : public BT8XXEMU::Flash
{
public:
	Flash(const BT8XXEMU_FlashParameters *params) : BT8XXEMU::Flash(&g_FlashVTable),
		Data(NULL), Size(0),
		m_DeepPowerDown(false), m_FileHandle(NULL), m_FileMapping(NULL), 
		m_ChipSelect(false), m_TransferCmd(0)
	{
		static_assert(offsetof(Flash, m_VTable) == 0, "Incompatible C++ ABI");

		m_Log = params->Log;
		m_UserContext = params->UserContext;
		m_PrintStd = true;

		Flash_debug("Create flash");

		m_StatusRegister = 0;

		m_WriteProtect = false;

		const wchar_t *const dataFilePath = params->DataFilePath;
		const size_t sizeBytes = params->SizeBytes;

		if (dataFilePath)
		{
			if (params->Persistent)
			{
				HANDLE fileHandle = CreateFileW(dataFilePath,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if (fileHandle == INVALID_HANDLE_VALUE)
				{
					log(BT8XXEMU_LogError, "Unable to access file '%ls'", dataFilePath);
				}
				else
				{
					LARGE_INTEGER fileSize;
					if (!GetFileSizeEx(fileHandle, &fileSize))
						fileSize.QuadPart = 0;
					LARGE_INTEGER size;
					size.QuadPart = sizeBytes;
					HANDLE fileMapping = CreateFileMappingW(fileHandle,
						NULL,
						PAGE_READWRITE,
						size.HighPart,
						size.LowPart,
						NULL);
					if (fileMapping == INVALID_HANDLE_VALUE)
					{
						log(BT8XXEMU_LogError, "Unable to map file '%ls'", dataFilePath);
						CloseHandle(fileHandle);
					}
					else
					{
						Data = (uint8_t *)(void *)MapViewOfFile(fileMapping,
							FILE_MAP_READ | FILE_MAP_WRITE,
							0, 0, 0);
						if (!Data)
						{
							log(BT8XXEMU_LogError, "Unable to map view of file '%ls'", dataFilePath);
							CloseHandle(fileMapping);
							CloseHandle(fileHandle);
						}
						else
						{
							Size = sizeBytes;
							m_FileHandle = fileHandle;
							m_FileMapping = fileMapping;
							fileSize.QuadPart = std::max(0LL, fileSize.QuadPart);
							if ((long long)sizeBytes > fileSize.QuadPart)
								memset(&Data[fileSize.QuadPart], 0xFF, sizeBytes - fileSize.QuadPart);
						}
					}
				}
			}
			else
			{
				if (!PathFileExistsW(dataFilePath))
				{
					log(BT8XXEMU_LogError, "Path to flash data does not exist '%ls'", dataFilePath);
				}
				else
				{
					int64_t fileSize = ::getFileSize(dataFilePath);
					if (fileSize == -1LL)
					{
						log(BT8XXEMU_LogError, "Unable to get flash data file size '%ls'", dataFilePath);
					}
					else
					{
					
						Data = new uint8_t[sizeBytes];
						Size = sizeBytes;
						size_t copySize = std::min((size_t)fileSize, sizeBytes);
						FILE *f = _wfopen(dataFilePath, L"rb");
						if (!f) log(BT8XXEMU_LogError, "Failed to open flash data file");
						else
						{
							size_t s = fread(Data, 1, copySize, f);
							if (s != copySize) log(BT8XXEMU_LogError, "Incomplete flash data file");
							else log(BT8XXEMU_LogMessage, "Loaded flash data file '%ls'", dataFilePath);
							if (fclose(f)) log(BT8XXEMU_LogError, "Error closing flash data file");
							if (sizeBytes > copySize)
								memset(&Data[copySize], 0xFF, sizeBytes - copySize);
						}
					}
				}
			}
		}

		if (!Data)
		{
			// Initialize regular empty RAM based flash device
			Data = new uint8_t[sizeBytes];
			memset(Data, 0xFF, sizeBytes);
			Size = sizeBytes;
		}

		if (params->Data)
		{
			// Copy user-provided data to flash
			size_t copySize = std::min(params->DataSizeBytes, sizeBytes);
			memcpy(Data, params->Data, copySize);
		}
	}

	~Flash()
	{
		Flash_debug("Destroy flash");
		std::unique_lock<std::mutex> lock(s_LogMutex);

		if (m_FileHandle)
		{
			// FlushViewOfFile(Data, Size);
			UnmapViewOfFile(Data);
			CloseHandle(m_FileMapping);
			CloseHandle(m_FileHandle);
		}
		else
		{
			delete Data;
		}
		Data = NULL;
		Size = 0;
	}

	void chipSelect(bool cs)
	{
		m_ChipSelect = cs;
		m_TransferCmd = 0;
		m_TransferNb = -1;
	}

	void writeProtect(bool wp)
	{
		m_WriteProtect = wp;
	}

	void hold(bool hold)
	{

	}

	uint8_t transfer(uint8_t data)
	{
		if (m_ChipSelect)
		{
			++m_TransferNb;
			if (m_DeepPowerDown)
			{
				if (!m_TransferNb && data == BTFLASH_CMD_RDP) /* Release from Deep Power Down */
				{
					Flash_debug("Release from Deep Power Down");
					m_DeepPowerDown = false;
				}
				else
				{
					log(BT8XXEMU_LogWarning, "Not processing flash commands while in Deep Power Down");
					return 0;
				}
			}
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
					Flash_debug("Write Status Register");
					; {
						if ((m_StatusRegister & BTFLASH_STATUS_SRWD_FLAG)
							&& m_WriteProtect)
						{
							log(BT8XXEMU_LogError, "Cannot write while in Hardware Protection Mode");
							return 0;
						}

						static const uint8_t mask =
							BTFLASH_STATUS_BP0_FLAG
							| BTFLASH_STATUS_BP1_FLAG
							| BTFLASH_STATUS_BP2_FLAG
							| BTFLASH_STATUS_BP3_FLAG
							| BTFLASH_STATUS_SRWD_FLAG;
						static const uint8_t invMask = ~mask;
						const uint8_t maskedWrite = data & mask;
						const uint8_t maskedReg = m_StatusRegister & invMask;
						m_StatusRegister = maskedWrite | maskedReg;
					}
					return 0;
				case BTFLASH_CMD_READ: /* Read Data */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_READ)");
					break;
				case BTFLASH_CMD_FAST_READ: /* Fast Read Data */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_FAST_READ)");
					break;
				case BTFLASH_CMD_2READ: /* 2x IO Read */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_2READ)");
					break;
				case BTFLASH_CMD_SE: /* Sector Erase */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_SE)");
					break;
				case BTFLASH_CMD_BE: /* Block Erase */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_BE)");
					break;
				case BTFLASH_CMD_CE_60: /* Chip Erase */
				case BTFLASH_CMD_CE_C7: /* Chip Erase */
					Flash_debug("Chip Erase");
					if (m_StatusRegister & BTFLASH_STATUS_WEL_FLAG)
					{
						memset(Data, 0xFF, Size);
						return 0;
					}
					else
					{
						log(BT8XXEMU_LogError, "Chip Erase requires Write Enable Latch to be set");
						break;
					}
				case BTFLASH_CMD_PP: /* Page Program */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_PP)");
					break;
				case BTFLASH_CMD_CP: /* Continuously Program mode */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_CP)");
					break;
				case BTFLASH_CMD_DP: /* Deep Power Down */
					Flash_debug("Deep Power Down");
					m_DeepPowerDown = true;
					return 0;
				case BTFLASH_CMD_RES: /* Read Electronic ID */
					Flash_debug("Read Electronic ID");
					goto ProcessCmdRes;
				case BTFLASH_CMD_REMS: /* Read Electronic Manufacturer and Device ID */
				case BTFLASH_CMD_REMS2: /* Read ID for 2x IO Mode */
					Flash_debug("Read Electronic Manufacturer and Device ID");
					goto ProcessCmdRems;
				case BTFLASH_CMD_ENSO: /* Enter Secured OTP */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_ENSO)");
					break;
				case BTFLASH_CMD_EXSO: /* Exit Secured OTP */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_EXSO)");
					break;
				case BTFLASH_CMD_RDSCUR: /* Read Security Register */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_RDSCUR)");
					break;
				case BTFLASH_CMD_WRSCUR: /* Write Security Register */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_WRSCUR)");
					break;
				case BTFLASH_CMD_ESRY: /* Enable SO to output RY/BY# */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_ESRY)");
					break;
				case BTFLASH_CMD_DSRY: /* Disable SO to output RY/BY# */
					log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_DSRY)");
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
			case BTFLASH_CMD_RES:
			ProcessCmdRes:
				// Repeatedly keeps returning the same value
				if (Size >= 8 * 1024 * 2014) return 0x16;
				else if (Size >= 4 * 1024 * 2014) return 0x15;
				else if (Size >= 2 * 1024 * 2014) return 0x14;
				else
				{
					log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
					break;
				}
			case BTFLASH_CMD_REMS:
			case BTFLASH_CMD_REMS2:
			ProcessCmdRems:
				// Repeatedly keeps returning the same values
				if (m_TransferNb % 1)
				{
					goto ProcessCmdRes;
				}
				else
				{
					return 0xC2;
				}
			default:
				log(BT8XXEMU_LogError, "Flash command (%x) exceeded transfer length", m_TransferCmd);
				break;
			}
		}
		return 0;
	}

	uint8_t *Data;
	size_t Size;

private:
	bool m_DeepPowerDown;

	HANDLE m_FileHandle;
	HANDLE m_FileMapping;

	bool m_ChipSelect;
	uint8_t m_TransferCmd;
	int m_TransferNb;

	bool m_WriteProtect; // WP# low, 0 -> WriteProtect true

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

size_t Flash_size(Flash *flash)
{
	return flash->Size;
}

BT8XXEMU_FlashVTable g_FlashVTable = {
	(void(*)(BT8XXEMU::Flash *))Flash_destroy,

	(uint8_t(*)(BT8XXEMU::Flash *, uint8_t))Flash_transfer,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_chipSelect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_writeProtect,
	(void(*)(BT8XXEMU::Flash *, bool))Flash_hold,

	(uint8_t *(*)(BT8XXEMU::Flash *))Flash_data,
	(size_t(*)(BT8XXEMU::Flash *))Flash_size
};

BT8XXEMU_EXPORT BT8XXEMU_Flash *__stdcall BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return NULL;
	}

	return new ::Flash(params);
}

int64_t getFileSize(const wchar_t* name)
{
	// https://stackoverflow.com/questions/8991192/check-filesize-without-opening-file-in-c

	HANDLE hFile = CreateFileW(name, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1LL; // error condition, could call GetLastError to find out more

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size))
	{
		CloseHandle(hFile);
		return -1LL; // error condition, could call GetLastError to find out more
	}

	CloseHandle(hFile);
	return size.QuadPart;
}

/* end of file */
