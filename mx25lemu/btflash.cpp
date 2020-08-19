/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26812) // Unscoped enum
#endif

#include "bt8xxemu.h"
#include "bt8xxemu_flash.h"

#include "btflash_defs.h"

/*
Name   Cmd  Implementation  Test
> (Up to 128Mbit)
RES         Ok              Ok
REMS        Ok              Ok
RDID        Ok              Ok
RDSFDP 5Ah  Ok              Ok
READ        Ok              Ok
FAST_READ   Ok              Ok
FASTDTRD    Ok              Ok
4READ  EBh  Ok              Ok
4DTRD  EDh  In progress     In progress
PP          Ok              Ok
WRSR        Ok
RDSR        Ok
WREN        Ok              Ok
WRDI        Ok
CE     C7h  Ok              Ok
SE          Ok              Ok
BE
> (From 256Mbit) (bottom 128Mbits are lowest address range)
RSTEN  66h  Ok
RST    99h  Ok
EN4B   B7h  Ok              Ok
EX4B        Ok              Ok
> Other
EXITQPIFFh  Ok (No-op)
*/

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
#include <assert.h>

#include <algorithm>
#include <mutex>

#ifdef _DEBUG  
#	define Flash_debug(message, ...) log(BT8XXEMU_LogMessage, "[[DEBUG]] " message, __VA_ARGS__)
#else
#	define Flash_debug(message, ...) do { } while (false)
#endif

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

#pragma pack(push, 1)
struct FlashSfdp
{
	union
	{
		uint8_t Data[0x70];
		struct 
		{
			union
			{
				struct
				{
					uint32_t SfdpSignature : 32;
					uint8_t SfdpMinorRev : 8;
					uint8_t SfdpMajorRev : 8;
					uint8_t NbParamHeaders : 8;
					uint8_t SfdpUnused_FFh : 8;


					uint8_t IdJedec : 8;
					uint8_t JedecParamTabMinorRev : 8;
					uint8_t JedecParamTabMajorRev : 8;
					uint8_t JedecParamTabLength : 8;

					uint32_t JedecParamTabPointer : 24;
					uint32_t JedecUnused_FFh : 8;


					uint8_t IdMacronix : 8;
					uint8_t MacronixParamTabMinorRev : 8;
					uint8_t MacronixParamTabMajorRev : 8;
					uint8_t MacronixParamTabLength : 8;

					uint32_t MacronixParamTabPointer : 24;
					uint32_t MacronixUnused_FFh : 8;

				};
				uint8_t Header[0x30];

			};
			union
			{
				struct
				{
					uint8_t BlockSectorEraseSizes : 2;
					uint8_t WriteGranularity : 1;
					uint8_t WriteEnableInstruction : 1;
					uint8_t WriteEnableOpCode : 1;
					uint8_t Unused1_111b : 3;

					uint8_t FourKbEraseOpCode : 8;

					uint8_t FastRead112 : 1;
					uint8_t AddressBytesArray : 2;
					uint8_t DoubleTransferRate : 1;
					uint8_t FastRead122 : 1;
					uint8_t FastRead144 : 1;
					uint8_t FastRead114 : 1;
					uint8_t Unused1_1b : 1;

					uint8_t Unused1_FFh : 8;


					uint32_t FlashMemoryDensity : 32;


					uint8_t FastReadWaitStates144 : 5;
					uint8_t FastReadModeBits144 : 3;

					uint8_t FastReadOpCode144 : 8;

					uint8_t FastReadWaitStates114 : 5;
					uint8_t FastReadModeBits114 : 3;

					uint8_t FastReadOpCode114 : 8;


					uint8_t FastReadWaitStates112 : 5;
					uint8_t FastReadModeBits112 : 3;

					uint8_t FastReadOpCode112 : 8;

					uint8_t FastReadWaitStates122 : 5;
					uint8_t FastReadModeBits122 : 3;

					uint8_t FastReadOpCode122 : 8;


					uint32_t FastRead222 : 1;
					uint32_t Unused2_111b : 3;
					uint32_t FastRead444 : 1;
					uint32_t Unused3_111b : 3;

					uint32_t Unused1_FFFFFFh : 24;


					uint8_t Unused10_FFh : 8;
					uint8_t Unused11_FFh : 8;

					uint8_t FastReadWaitStates222 : 5;
					uint8_t FastReadModeBits222 : 3;

					uint8_t FastReadOpCode222 : 8;


					uint8_t Unused12_FFh : 8;
					uint8_t Unused13_FFh : 8;

					uint8_t FastReadWaitStates444 : 5;
					uint8_t FastReadModeBits444 : 3;

					uint8_t FastReadOpCode444 : 8;


					uint8_t SectorType1Size : 8;
					uint8_t SectorType1EraseOpCode : 8;
					uint8_t SectorType2Size : 8;
					uint8_t SectorType2EraseOpCode : 8;
					uint8_t SectorType3Size : 8;
					uint8_t SectorType3EraseOpCode : 8;
					uint8_t SectorType4Size : 8;
					uint8_t SectorType4EraseOpCode : 8;

				};
				uint8_t JedecTable[0x60 - 0x30];

			};
			union
			{
				struct
				{
					uint16_t VccSupplyMax : 16;
					uint16_t VccSupplyMin : 16;


					uint16_t HwResetPin : 1;
					uint16_t HwHoldPin : 1;
					uint16_t DeepPowerDownMode : 1;
					uint16_t SwReset : 1;
					uint16_t SwResetOpcode : 8;
					uint16_t ProgramSuspendResume : 1;
					uint16_t EraseSuspendResume : 1;
					uint16_t Unused2_1b : 1;
					uint16_t WrapAroundReadMode : 1;

					uint16_t WrapAroundReadModeOpCode : 8;
					uint16_t WrapAroundReadDataLength : 8;


					uint16_t IndividualBlockLock : 1;
					uint16_t IndividualBlockLockBit : 1;
					uint16_t IndividualBlockLockOpCode : 8;
					uint16_t IndividualBlockLockVolatile : 1;
					uint16_t SecuredOTP : 1;
					uint16_t ReadLock : 1;
					uint16_t PermanentLock : 1;
					uint16_t Unused1_11b : 2;

					uint16_t Unused1_FFFFh : 16;


					uint32_t Unused1_FFFFFFFFh : 32;

				};
				uint8_t MacronixTable[0x70 - 0x60];

			};
		};
	};
};
#pragma pack(pop)

static const union
{
	FlashSfdp Format;
	uint8_t Data[256] = { 
		83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255,
		194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		229, 32, 184, 255, 255, 255, 255, 3, 68, 235, 0, 255, 0, 255, 4, 187, 
		238, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 12, 32, 15, 82, 
		16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		0, 54, 0, 39, 244, 79, 255, 255, 217, 200, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255,
		194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		229, 32, 184, 255, 255, 255, 255, 3, 68, 235, 0, 255, 0, 255, 4, 187, 
		238, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 12, 32, 15, 82, 
		16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		0, 54, 0, 39, 244, 79, 255, 255, 217, 200, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
} s_SfdpMX25L64;

static const union
{
	FlashSfdp Format;
	uint8_t Data[256] = { 
		83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255, 
		194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		229, 32, 243, 255, 255, 255, 255, 15, 68, 235, 8, 107, 8, 59, 4, 187, 
		254, 255, 255, 255, 255, 255, 0, 255, 255, 255, 68, 235, 12, 32, 15, 82, 
		16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		0, 54, 0, 39, 157, 249, 192, 100, 133, 203, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 194, 245, 8, 11, 
		5, 2, 5, 7, 0, 0, 15, 24, 44, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
} s_SfdpMX25L256;

class Flash : public BT8XXEMU::Flash
{
public:
	Flash(const BT8XXEMU_FlashParameters *params) : BT8XXEMU::Flash(&g_FlashVTable),
		Data(NULL), Size(0),
		m_DeepPowerDown(false), m_FileHandle(NULL), m_FileMapping(NULL), 
		m_LastSignal(0xFF)
	{
		static_assert(offsetof(Flash, m_VTable) == 0, "Incompatible C++ ABI");
		assert(s_SfdpMX25L256.Format.Unused2_1b == 1);
		assert(s_SfdpMX25L64.Format.Unused2_1b == 1);
		assert(s_SfdpMX25L256.Format.Unused1_11b == 3);
		assert(s_SfdpMX25L64.Format.Unused1_11b == 3);
		assert(s_SfdpMX25L256.Format.Unused1_FFFFFFFFh == 0xFFFFFFFF);
		assert(s_SfdpMX25L64.Format.Unused1_FFFFFFFFh == 0xFFFFFFFF);

		m_Log = params->Log;
		m_UserContext = params->UserContext;
		m_PrintStd = params->StdOut;

		Flash_debug("Create flash");

		m_StatusRegister = 0;
		m_ConfigurationRegister = 0;
		m_ExtendedAddressRegister = 0;
		m_ResetEnable = false;

		// m_StatusRegister = BTFLASH_STATUS_QE_FLAG; // TEST
		// m_SelectState = BTFLASH_STATE_4READ_ADDR; // TEST: The 0xFF signals at BT815 boot-up will cause this state to reset

		m_WriteProtect = false;

		const wchar_t *const dataFilePath = params->DataFilePath[0] ? params->DataFilePath : NULL;
		const size_t sizeBytes = (params->SizeBytes + 65535) & (~(size_t)0xFFFF);
		// TODO: Always round up size to power-of-2

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
					if (fileMapping == INVALID_HANDLE_VALUE || !fileMapping)
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

		m_ExtendedAddressing = Size > (1 << 24);

		if (m_ExtendedAddressing) memcpy(&m_SfdpData, &s_SfdpMX25L256.Data, 256);
		else memcpy(&m_SfdpData, &s_SfdpMX25L64.Data, 256);

		m_Sfdp.FlashMemoryDensity = (Size * 8) - 1;
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
		if (cs == false)
		{
			switch (m_RunState)
			{
			case BTFLASH_STATE_CS_HIGH_COMMAND:
				/*
				WREN, WRDI, WRSR, SE/SE4B, BE32K/BE32K4B, BE/BE4B, CE, PP/PP4B, 4PP/4PP4B, DP,
				ENSO, EXSO, WRSCUR, EN4B, EX4B, WPSEL, GBLK, GBULK, SPBLK, SUSPEND, RESUME, NOP, RSTEN,
				RST, EQIO, RSTQIO
				*/
				switch (m_DelayedCommand)
				{
				case BTFLASH_CMD_CE_C7:
					Flash_debug("Chip Erase (Execute)");
					m_StatusRegister &= ~BTFLASH_STATUS_WEL_FLAG;
					memset(Data, 0xFF, Size);
					break;
				case BTFLASH_CMD_EN4B:
					Flash_debug("Enter 4-byte mode (Execute)");
					m_ConfigurationRegister |= BTFLASH_CONFIGURATION_4BYTE;
					break;
				case BTFLASH_CMD_EX4B:
					Flash_debug("Exit 4-byte mode (Execute)");
					m_ConfigurationRegister &= ~BTFLASH_CONFIGURATION_4BYTE;
					break;
				case BTFLASH_CMD_RSTEN:
					Flash_debug("Reset Enable (Execute)");
					m_ResetEnable = true;
					break;
				case BTFLASH_CMD_RST:
					Flash_debug("Reset (Execute)");
					m_SelectState = BTFLASH_STATE_COMMAND;
					m_StatusRegister &= 0xFC; // Only WIP and WEL are volatile
					m_ConfigurationRegister &= 0x08; // TB is non-volatile, OTP
					m_ExtendedAddressRegister = 0;
					m_ResetEnable = false;
					break;
				case BTFLASH_CMD_SE:
					Flash_debug("Sector Erase (Execute)");
					m_StatusRegister &= ~BTFLASH_STATUS_WEL_FLAG;
					if (((size_t)(m_DelayedCommandAddr & ~0xFFFUL) + 4096) > Size)
					{
						log(BT8XXEMU_LogError, "Erase address exceeds address space");
					}
					else
					{
						memset(&Data[m_DelayedCommandAddr & ~0xFFFUL], 0xFF, 4096);
					}
					break;
				default:
					log(BT8XXEMU_LogError, "Unknown instruction on CS high");
					break;
				}
				break;
			case BTFLASH_STATE_PP_READ:
			case BTFLASH_STATE_4PP_READ:
				if (m_BufferBits)
				{
					log(BT8XXEMU_LogError, "CS must go high on byte boundary, PP instruction rejected");
				}
				else
				{
					Flash_debug("Page Program (Execute)");
					m_StatusRegister &= ~BTFLASH_STATUS_WEL_FLAG;
					const size_t addr = m_DelayedCommandAddr;
					size_t at = addr & 0xFF;
					const size_t page = addr - at;
					if ((page + 256) > Size)
					{
						log(BT8XXEMU_LogError, "Program address exceeds address space");
					}
					else
					{
						for (int i = 0; i < m_PageProgramSize; ++i)
						{
							Data[page + at] &= m_PageProgramData[i]; // Set only the 0s using &
							++at;
							at &= 0xFF;
						}
					}
				}
				break;
			}
		}

		m_RunState = m_SelectState; // COMMAND
		m_NextState = BTFLASH_STATE_BLANK;
		// m_ClockEdge = BTFLASH_CLOCK_RISING;
		m_SignalInMask = cs ? BTFLASH_SPI4_MASK_SI : 0;
		m_SignalOutMask = cs ? BTFLASH_SPI4_MASK_SO : 0;
		m_BufferBits = 0;
		m_OutBufferBits = 0;
	}

	void writeProtect(bool wp)
	{
		m_WriteProtect = wp;
	}

private:
	inline bool using4ByteAddress()
	{
		return (m_ExtendedAddressing && (m_ConfigurationRegister & BTFLASH_CONFIGURATION_4BYTE));
	}

	inline bool usingExtendedAddressRegister()
	{
		return (m_ExtendedAddressing && !(m_ConfigurationRegister & BTFLASH_CONFIGURATION_4BYTE));
	}

private:
	int m_SelectState = BTFLASH_STATE_COMMAND;
	int m_RunState;
	int m_NextState;
	uint8_t m_LastSignal;
	uint8_t m_SignalInMask;
	uint8_t m_SignalOutMask;

	union
	{
		uint8_t m_BufferU8;
		uint32_t m_BufferU32;
		uint64_t m_BufferU64;
	};
	int m_BufferBits;

	union
	{
		uint8_t m_OutBufferU8;
		uint16_t m_OutBufferU16;
		uint32_t m_OutBufferU32;
		uint64_t m_OutBufferU64;
		struct
		{
			uint8_t m_OutBufferU8A;
			uint8_t m_OutBufferU8B;
			uint8_t m_OutBufferU8C;
			uint8_t m_OutBufferU8D;
			uint8_t m_OutBufferU8E;
			uint8_t m_OutBufferU8F;
			uint8_t m_OutBufferU8G;
			uint8_t m_OutBufferU8H;
		};
	};
	int m_OutBufferBits;

	const uint8_t *m_OutArray = NULL;
	size_t m_OutArraySize;
	size_t m_OutArrayAt;
	bool m_OutArrayLoop;

	bool m_ExtendedAddressing;

public:
	inline uint8_t maskOutSignal(uint8_t signal, uint8_t signalOut)
	{
		return (signalOut & m_SignalOutMask)
			| (signal & (~m_SignalOutMask));
	}

	uint8_t transferSpi4(uint8_t signal)
	{
		// Detect chip select changes (both rising and falling edge)
		bool cs = !(signal & BTFLASH_SPI4_CS);
		if ((signal & BTFLASH_SPI4_CS) != (m_LastSignal & BTFLASH_SPI4_CS)) // CS
		{
			chipSelect(cs);
		}

		// Detect write protect changes
		bool wp = (!(m_StatusRegister & BTFLASH_STATUS_QE_FLAG)) && !(signal & BTFLASH_SPI4_WP);
		if (wp != m_WriteProtect)
		{
			writeProtect(wp);
		}

		// Process when selected
		if (cs)
		{
			// Detect clock rising and falling edge
			if ((signal & BTFLASH_SPI4_SCK) != (m_LastSignal & BTFLASH_SPI4_SCK)) // Clock
			{
				// Process clock on selected edge
				if (signal & BTFLASH_SPI4_SCK)
				{
					m_LastSignal = maskOutSignal(signal, clockRising(signal));
					// printf("> %i\n", (m_LastSignal & 2));
					return m_LastSignal;
				}
				else
				{
					m_LastSignal = maskOutSignal(signal, clockFalling(signal));
					// printf("> %i\n", (m_LastSignal & 2));
					return m_LastSignal;
				}
			}
		}

		m_LastSignal = maskOutSignal(signal, m_LastSignal);
		// printf("> %i\n", (m_LastSignal & 2));
		return m_LastSignal;
	}

	uint8_t clockRising(uint8_t signal)
	{
		switch (m_RunState)
		{
		case BTFLASH_STATE_COMMAND:
			return stateCommand(signal);
		case BTFLASH_STATE_WRSR:
			return stateWRSR(signal);
		case BTFLASH_STATE_REMS_ADDR:
			return stateREMSAddr(signal);
		case BTFLASH_STATE_RDSFDP_ADDR:
			return stateRDSFDPAddr(signal);
		case BTFLASH_STATE_READ_ADDR:
			return stateREADAddr(signal);
		case BTFLASH_STATE_FAST_READ_ADDR:
			return stateFAST_READAddr(signal);
		case BTFLASH_STATE_FASTDTRD_ADDR:
			return stateFASTDTRDAddr(signal);
		case BTFLASH_STATE_4READ_ADDR:
			return state4READAddr(signal);
		case BTFLASH_STATE_4READ_PE:
			return state4READPerfEnh(signal);
		case BTFLASH_STATE_4DTRD_ADDR:
			return state4READAddr(signal);
		case BTFLASH_STATE_4DTRD_PE:
			return state4READPerfEnh(signal);
		case BTFLASH_STATE_PP_ADDR:
			return statePPAddr(signal);
		case BTFLASH_STATE_PP_READ:
			return statePPRead(signal);
		case BTFLASH_STATE_SE_ADDR:
			return stateSEAddr(signal);
		case BTFLASH_STATE_4PP_ADDR:
			return state4PPAddr(signal);
		case BTFLASH_STATE_4PP_READ:
			return state4PPRead(signal);
		case BTFLASH_STATE_OUT_U8_ARRAY_DT:
			return stateWriteOutU8Array();
		case BTFLASH_STATE_NEXT:
			m_RunState = m_NextState;
			m_NextState = BTFLASH_STATE_BLANK;
			break;
		case BTFLASH_STATE_CS_HIGH_COMMAND:
			log(BT8XXEMU_LogWarning, "CS must go high exactly on the command byte boundary, instruction rejected");
			m_RunState = BTFLASH_STATE_BLANK;
			break;
		case BTFLASH_STATE_UNSUPPORTED:
		case BTFLASH_STATE_BLANK:
		case BTFLASH_STATE_OUT_U8:
		case BTFLASH_STATE_OUT_U16:
		case BTFLASH_STATE_OUT_U24:
		case BTFLASH_STATE_OUT_U32:
		case BTFLASH_STATE_OUT_U8_ARRAY:
			// Silent no-op
			break;
		case BTFLASH_STATE_UNKNOWN:
		default:
			log(BT8XXEMU_LogError, "Unknown state");
			break;
		}
		return m_LastSignal;
	}

	uint8_t clockFalling(uint8_t signal)
	{
		switch (m_RunState)
		{
		case BTFLASH_STATE_BLANK:
			// Allow falling edge for blank response
			m_RunState = BTFLASH_STATE_UNKNOWN;
			break;
		case BTFLASH_STATE_OUT_U8:
			if (isWritingOutBuffer())
			{
				uint8_t res = writeOutU8();
				if (!isWritingOutBuffer())
				{
					if (m_NextState == BTFLASH_STATE_OUT_U8)
					{
						m_OutBufferBits = 8;
					}
					else
					{
						m_RunState = m_NextState;
						m_NextState = BTFLASH_STATE_BLANK;
					}
				}
				return res;
			}
			else
			{
				log(BT8XXEMU_LogError, "Output state invalid without buffer");
			}
			break;
		case BTFLASH_STATE_OUT_U16:
			if (isWritingOutBuffer())
			{
				uint8_t res = writeOutU16();
				if (!isWritingOutBuffer())
				{
					if (m_NextState == BTFLASH_STATE_OUT_U16)
					{
						m_OutBufferBits = 16;
					}
					else
					{
						m_RunState = m_NextState;
						m_NextState = BTFLASH_STATE_BLANK;
					}
				}
				return res;
			}
			else
			{
				log(BT8XXEMU_LogError, "Output state invalid without buffer");
			}
			break;
		case BTFLASH_STATE_OUT_U24:
		case BTFLASH_STATE_OUT_U32:
			if (isWritingOutBuffer())
			{
				uint8_t res = writeOutU32();
				if (!isWritingOutBuffer())
				{
					if (m_NextState == BTFLASH_STATE_OUT_U24)
					{
						m_OutBufferBits = 24;
					}
					else if(m_NextState == BTFLASH_STATE_OUT_U32)
					{
						m_OutBufferBits = 32;
					}
					else
					{
						m_RunState = m_NextState;
						m_NextState = BTFLASH_STATE_BLANK;
					}
				}
				return res;
			}
			else
			{
				log(BT8XXEMU_LogError, "Output state invalid without buffer");
			}
			break;
		case BTFLASH_STATE_OUT_U8_ARRAY:
		case BTFLASH_STATE_OUT_U8_ARRAY_DT:
			return stateWriteOutU8Array();
		case BTFLASH_STATE_NEXT:
			m_RunState = m_NextState;
			m_NextState = BTFLASH_STATE_BLANK;
			break;
		case BTFLASH_STATE_FASTDTRD_ADDR:
			return stateFASTDTRDAddr(signal);
		case BTFLASH_STATE_4DTRD_ADDR:
			return state4READAddr(signal);
		case BTFLASH_STATE_4DTRD_PE:
			return state4READPerfEnh(signal);
		case BTFLASH_STATE_COMMAND:
		case BTFLASH_STATE_WRSR:
		case BTFLASH_STATE_REMS_ADDR:
		case BTFLASH_STATE_RDSFDP_ADDR:
		case BTFLASH_STATE_READ_ADDR:
		case BTFLASH_STATE_FAST_READ_ADDR:
		case BTFLASH_STATE_4READ_ADDR:
		case BTFLASH_STATE_4READ_PE:
		case BTFLASH_STATE_PP_ADDR:
		case BTFLASH_STATE_PP_READ:
		case BTFLASH_STATE_SE_ADDR:
		case BTFLASH_STATE_4PP_ADDR:
		case BTFLASH_STATE_4PP_READ:
		case BTFLASH_STATE_UNSUPPORTED:
		case BTFLASH_STATE_CS_HIGH_COMMAND:
			// Silent no-op
			break;
		case BTFLASH_STATE_UNKNOWN:
		default:
			log(BT8XXEMU_LogError, "Unknown state");
			break;
		}
		return m_LastSignal;
	}

	uint8_t stateWriteOutU8Array()
	{
		if (!isWritingOutBuffer())
		{
			if (m_OutArrayAt >= m_OutArraySize)
			{
				if (m_OutArrayLoop)
				{
					// Continue loop
					m_OutArrayAt -= m_OutArraySize;
				}
				else
				{
					// End
					m_RunState = m_NextState;
					return m_LastSignal;
				}
			}

			// Start next byte
			Flash_debug("Data[%i]: 0x%x (%i)",
				(int)m_OutArrayAt,
				(int)m_OutArray[m_OutArrayAt],
				(int)m_OutArray[m_OutArrayAt]);
			startWriteU8(m_OutArray[m_OutArrayAt]);
			++m_OutArrayAt;
		}

		// Send bits
		return writeOutU8();
	}
	
	bool isWritingOutBuffer()
	{
		return m_OutBufferBits > 0;
	}

	void startWriteU8(uint8_t data)
	{
		m_OutBufferU8 = data;
		m_OutBufferBits = 8;
	}

	void startWrite2U8AsU16(uint8_t a, uint8_t b)
	{
		m_OutBufferU8A = b;
		m_OutBufferU8B = a;
		m_OutBufferBits = 16;
	}

	void startWrite3U8AsU32(uint8_t a, uint8_t b, uint8_t c)
	{
		m_OutBufferU8A = c;
		m_OutBufferU8B = b;
		m_OutBufferU8C = a;
		m_OutBufferBits = 24;
	}

	uint8_t writeOutU8()
	{
		if (m_SignalOutMask == BTFLASH_SPI4_MASK_SO)
		{
			uint8_t res = ((m_OutBufferU8 >> (m_OutBufferBits - 1)) << 1) & BTFLASH_SPI4_MASK_SO;
			// printf("> SO %i\n", res & 2);
			--m_OutBufferBits;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D4)
		{
			uint8_t res = (m_OutBufferU8 >> (m_OutBufferBits - 4)) & BTFLASH_SPI4_MASK_D4;
			m_OutBufferBits -= 4;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D2)
		{
			uint8_t res = (m_OutBufferU8 >> (m_OutBufferBits - 2)) & BTFLASH_SPI4_MASK_D2;
			m_OutBufferBits -= 2;
			return res;
		}
		else
		{
			log(BT8XXEMU_LogError, "Invalid signal output directions");
			return m_LastSignal;
		}
	}

	uint8_t writeOutU16()
	{
		if (m_SignalOutMask == BTFLASH_SPI4_MASK_SO)
		{
			uint8_t res = ((m_OutBufferU16 >> (m_OutBufferBits - 1)) << 1) & BTFLASH_SPI4_MASK_SO;
			// printf("> SO %i\n", res & 2);
			--m_OutBufferBits;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D4)
		{
			uint8_t res = (m_OutBufferU16 >> (m_OutBufferBits - 4)) & BTFLASH_SPI4_MASK_D4;
			m_OutBufferBits -= 4;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D2)
		{
			uint8_t res = (m_OutBufferU16 >> (m_OutBufferBits - 2)) & BTFLASH_SPI4_MASK_D2;
			m_OutBufferBits -= 2;
			return res;
		}
		else
		{
			log(BT8XXEMU_LogError, "Invalid signal output directions");
			return m_LastSignal;
		}
	}

	uint8_t writeOutU32()
	{
		if (m_SignalOutMask == BTFLASH_SPI4_MASK_SO)
		{
			uint8_t res = ((m_OutBufferU32 >> (m_OutBufferBits - 1)) << 1) & BTFLASH_SPI4_MASK_SO;
			--m_OutBufferBits;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D4)
		{
			uint8_t res = (m_OutBufferU32 >> (m_OutBufferBits - 4)) & BTFLASH_SPI4_MASK_D4;
			m_OutBufferBits -= 4;
			return res;
		}
		else if (m_SignalOutMask == BTFLASH_SPI4_MASK_D2)
		{
			uint8_t res = (m_OutBufferU32 >> (m_OutBufferBits - 2)) & BTFLASH_SPI4_MASK_D2;
			m_OutBufferBits -= 2;
			return res;
		}
		else
		{
			log(BT8XXEMU_LogError, "Invalid signal output directions");
			return m_LastSignal;
		}
	}
	
	bool readU8Spi1(uint8_t signal)
	{
		// Read 1 bit at a time until a U8 is read
		m_BufferU8 = (m_BufferU8 << 1)
			| (BTFLASH_SPI4_GET_SI(signal));
		++m_BufferBits;
		return m_BufferBits == 8;
	}

	bool readU8Spi4(uint8_t signal)
	{
		// Read 4 bits at a time until a U8 is read
		m_BufferU8 = (m_BufferU8 << 4)
			| (BTFLASH_SPI4_GET_D4(signal));
		m_BufferBits += 4;
		return m_BufferBits == 8;
	}

	bool readU24Spi1AsU32(uint8_t signal)
	{
		// Read 1 bit at a time until a U24 is read
		m_BufferU32 = (m_BufferU32 << 1)
			| (BTFLASH_SPI4_GET_SI(signal));
		++m_BufferBits;
		return m_BufferBits == 24;
	}

	bool readU24Spi1AsU32SkipLsb(uint8_t signal, int dummyBits)
	{
		// Read 1 bit at a time until a U24 is read
		// Skip dummy bits bits after the U24
		if (m_BufferBits < 24)
		{
			m_BufferU32 = (m_BufferU32 << 1)
				| (BTFLASH_SPI4_GET_SI(signal));
		}
		++m_BufferBits;
		return m_BufferBits == 24 + dummyBits;
	}

	bool readAddressSpi1AsU32SkipLsb(uint8_t signal, int dummyBits)
	{
		int addressBits = using4ByteAddress() ? 32 : 24;
		if (m_BufferBits < addressBits)
		{
			m_BufferU32 = (m_BufferU32 << 1)
				| (BTFLASH_SPI4_GET_SI(signal));
		}
		++m_BufferBits;
		if (m_BufferBits == addressBits + dummyBits)
		{
			if (usingExtendedAddressRegister())
				m_BufferU32 = (m_BufferU32 & 0xFFFFFF) | (((uint32_t)m_ExtendedAddressRegister) << 24);
			else if (!using4ByteAddress())
				m_BufferU32 = (m_BufferU32 & 0xFFFFFF);
			return true;
		}
		return false;
	}

	bool readAddressSpi4AsU32SkipLsb(uint8_t signal, int dummyBits)
	{
		int addressBits = using4ByteAddress() ? 32 : 24;
		if (m_BufferBits < addressBits)
		{
			m_BufferU32 = (m_BufferU32 << 4)
				| (BTFLASH_SPI4_GET_D4(signal));
		}
		m_BufferBits += 4;
		if (m_BufferBits == (addressBits + dummyBits))
		{
			if (usingExtendedAddressRegister())
				m_BufferU32 = (m_BufferU32 & 0xFFFFFF) | (((uint32_t)m_ExtendedAddressRegister) << 24);
			else if (!using4ByteAddress())
				m_BufferU32 = (m_BufferU32 & 0xFFFFFF);
			return true;
		}
		return false;
	}

	bool readU8Spi4SkipLsb(uint8_t signal, int dummyBits)
	{
		if (m_BufferBits < 8)
		{
			m_BufferU8 = (m_BufferU32 << 4)
				| (BTFLASH_SPI4_GET_D4(signal));
		}
		m_BufferBits += 4;
		return (m_BufferBits == (8 + dummyBits));
	}

	inline uint8_t manufacturerId()
	{
		return 0xc2;
	}

	inline uint8_t deviceId()
	{
		if (Size >= 2UL * 1024UL * 1024UL * 1024UL)
		{
			log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
			return 0xFF;
		}
		else if (Size >= 1024 * 1024 * 1024) return 0x1D;
		else if (Size >= 512 * 1024 * 1024) return 0x1C;
		else if (Size >= 256 * 1024 * 1024) return 0x1B;
		else if (Size >= 128 * 1024 * 1024) return 0x1A;
		else if (Size >= 64 * 1024 * 1024) return 0x19;
		else if (Size >= 32 * 1024 * 1024) return 0x18; // 256 Mbit
		else if (Size >= 16 * 1024 * 1024) return 0x17;
		else if (Size >= 8 * 1024 * 1024) return 0x16; // 64 Mbit
		else if (Size >= 4 * 1024 * 1024) return 0x15;
		else if (Size >= 2 * 1024 * 1024) return 0x14;
		else if (Size >= 1024 * 1024) return 0x13;
		else if (Size >= 512 * 1024) return 0x12;
		else if (Size >= 256 * 1024) return 0x11;
		else if (Size >= 128 * 1024) return 0x10;
		else if (Size >= 64 * 1024) return 0x0F;
		else if (Size >= 32 * 1024) return 0x0E;
		else if (Size >= 16 * 1024) return 0x0D;
		else if (Size >= 8 * 1024) return 0x0C;
		else if (Size >= 4 * 1024) return 0x0B;
		else if (Size >= 2 * 1024) return 0x0A;
		else if (Size >= 1024) return 0x09;
		else
		{
			log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
			return 0xFF;
		}
	}

	inline uint8_t memoryType()
	{
		return 0x20;
	}

	inline uint8_t memoryDensity()
	{
		if (Size >= 2UL * 1024UL * 1024UL * 1024UL)
		{
			log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
			return 0xFF;
		}
		else if (Size >= 1024 * 1024 * 1024) return 0x1E;
		else if (Size >= 512 * 1024 * 1024) return 0x1D;
		else if (Size >= 256 * 1024 * 1024) return 0x1C;
		else if (Size >= 128 * 1024 * 1024) return 0x1B;
		else if (Size >= 64 * 1024 * 1024) return 0x1A;
		else if (Size >= 32 * 1024 * 1024) return 0x19; // 256 Mbit
		else if (Size >= 16 * 1024 * 1024) return 0x18;
		else if (Size >= 8 * 1024 * 1024) return 0x17; // 64 Mbit
		else if (Size >= 4 * 1024 * 1024) return 0x16;
		else if (Size >= 2 * 1024 * 1024) return 0x15;
		else if (Size >= 1024 * 1024) return 0x14;
		else if (Size >= 512 * 1024) return 0x13;
		else if (Size >= 256 * 1024) return 0x12;
		else if (Size >= 128 * 1024) return 0x11;
		else if (Size >= 64 * 1024) return 0x10;
		else if (Size >= 32 * 1024) return 0x0F;
		else if (Size >= 16 * 1024) return 0x0E;
		else if (Size >= 8 * 1024) return 0x0D;
		else if (Size >= 4 * 1024) return 0x0C;
		else if (Size >= 2 * 1024) return 0x0B;
		else if (Size >= 1024) return 0x0A;
		else
		{
			log(BT8XXEMU_LogError, "Unavailable device size %u", Size);
			return 0xFF;
		}
	}

	uint8_t stateCommand(uint8_t signal)
	{
		if (readU8Spi1(signal))
		{
			// Check command and switch to next state
			uint8_t command = m_BufferU8;
			m_BufferBits = 0;
			if (m_DeepPowerDown)
			{
				if (command == BTFLASH_CMD_RDP) /* Release from Deep Power Down */
				{
					Flash_debug("Release from Deep Power Down");
					m_DeepPowerDown = false;
				}
				else
				{
					log(BT8XXEMU_LogWarning, "Not processing flash commands while in Deep Power Down");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
			}
			switch (command)
			{
			case BTFLASH_CMD_WREN: /* Write Enable */
				Flash_debug("Write Enable");
				m_StatusRegister |= BTFLASH_STATUS_WEL_FLAG;
				m_RunState = BTFLASH_STATE_BLANK;
				return m_LastSignal;
			case BTFLASH_CMD_WRDI: /* Write Disable */
				Flash_debug("Write Disable");
				m_StatusRegister &= ~BTFLASH_STATUS_WEL_FLAG;
				m_RunState = BTFLASH_STATE_BLANK;
				return m_LastSignal;
			case BTFLASH_CMD_RDID: /* Read Identification */
				Flash_debug("Read Identification");
				m_RunState = BTFLASH_STATE_OUT_U24;
				// m_NextState = BTFLASH_STATE_OUT_U24; // Does not repeat continuously
				startWrite3U8AsU32(manufacturerId(), memoryType(), memoryDensity());
				return m_LastSignal;
			case BTFLASH_CMD_RDSR: /* Read Status Register */
				Flash_debug("Read Status Register");
				m_RunState = BTFLASH_STATE_OUT_U8;
				startWriteU8(m_StatusRegister);
				Flash_debug("RDSR 0x%x", m_StatusRegister);
				return m_LastSignal;
			case BTFLASH_CMD_WRSR: /* Write Status Register */
				Flash_debug("Write Status Register");
				if (!(m_StatusRegister & BTFLASH_STATUS_WEL_FLAG))
				{
					log(BT8XXEMU_LogError, "Write Enable Latch must be set before writing Status Register");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
				if ((m_StatusRegister & BTFLASH_STATUS_SRWD_FLAG)
					&& m_WriteProtect)
				{
					log(BT8XXEMU_LogError, "Cannot write Status Register while in Hardware Protection Mode");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
				m_RunState = BTFLASH_STATE_WRSR;
				return m_LastSignal;
			case BTFLASH_CMD_FASTDTRD: /* Fast DT Read */
				Flash_debug("Fast DT Read");
				if (m_ExtendedAddressing)
				{
					log(BT8XXEMU_LogError, "Fast DT Read not supported with extended addressing");
					m_RunState = BTFLASH_STATE_UNSUPPORTED;
				}
				else
				{
					m_RunState = BTFLASH_STATE_NEXT;
					m_NextState = BTFLASH_STATE_FASTDTRD_ADDR;
				}
				return m_LastSignal;
			case BTFLASH_CMD_2DTRD: /* 2x IO DT Read */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_2DTRD)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_4DTRD: /* 4x IO DT Read */
				Flash_debug("4x IO DT Read");
				// log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_4DTRD)");
				// m_RunState = BTFLASH_STATE_UNSUPPORTED;
				if (m_ExtendedAddressing)
				{
					log(BT8XXEMU_LogError, "4x IO DT Read not supported with extended addressing");
					m_RunState = BTFLASH_STATE_UNSUPPORTED;
				}
				else if (!(m_StatusRegister & BTFLASH_STATUS_QE_FLAG))
				{
					log(BT8XXEMU_LogError, "Quad Enable must be flagged to use 4x IO DT Read");
					m_RunState = BTFLASH_STATE_IGNORE;
				}
				else
				{
					m_RunState = BTFLASH_STATE_NEXT;
					m_NextState = BTFLASH_STATE_4DTRD_ADDR;
				}
				return m_LastSignal;
			case BTFLASH_CMD_READ: /* Read Data */
				Flash_debug("Read Data");
				m_RunState = BTFLASH_STATE_READ_ADDR;
				return m_LastSignal;
			case BTFLASH_CMD_FAST_READ: /* Fast Read Data */
				Flash_debug("Fast Read Data");
				m_RunState = BTFLASH_STATE_FAST_READ_ADDR;
				return m_LastSignal;
			case BTFLASH_CMD_RDSFDP: /* Read SFDP */
				Flash_debug("Read SFDP");
				m_RunState = BTFLASH_STATE_RDSFDP_ADDR;
				return m_LastSignal;
			case BTFLASH_CMD_2READ: /* 2x IO Read */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_2READ)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_4READ: /* 4x IO Read */
				Flash_debug("4x IO Read");
				if (!(m_StatusRegister & BTFLASH_STATUS_QE_FLAG))
				{
					log(BT8XXEMU_LogError, "Quad Enable must be flagged to use 4x IO Read");
					m_RunState = BTFLASH_STATE_IGNORE;
				}
				else
				{
					m_SignalOutMask = BTFLASH_SPI4_MASK_NONE;
					m_RunState = BTFLASH_STATE_4READ_ADDR;
				}
				return m_LastSignal;
			case BTFLASH_CMD_4PP: /* Quad Page Program */
				Flash_debug("Quad Page Program");
				if (!(m_StatusRegister & BTFLASH_STATUS_QE_FLAG))
				{
					log(BT8XXEMU_LogError, "Quad Enable must be flagged to use 4x IO Read");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
				else if (!(m_StatusRegister & BTFLASH_STATUS_WEL_FLAG))
				{
					log(BT8XXEMU_LogError, "Page Program requires Write Enable Latch to be set");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
				else
				{
					m_SignalOutMask = BTFLASH_SPI4_MASK_NONE;
					m_DelayedCommand = BTFLASH_CMD_4PP;
					m_RunState = BTFLASH_STATE_4PP_ADDR;
					return m_LastSignal;
				}
			case BTFLASH_CMD_SE: /* Sector Erase */
				Flash_debug("Sector Erase");
				if (m_StatusRegister & BTFLASH_STATUS_WEL_FLAG)
				{
					m_DelayedCommand = BTFLASH_CMD_SE;
					m_RunState = BTFLASH_STATE_SE_ADDR;
					return m_LastSignal;
				}
				else
				{
					log(BT8XXEMU_LogError, "Sector Erase requires Write Enable Latch to be set");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
			case BTFLASH_CMD_BE: /* Block Erase */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_BE)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_BE32K: /* Block Erase 32kB */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_BE32K)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_CE_60: /* Chip Erase */
			case BTFLASH_CMD_CE_C7: /* Chip Erase */
				Flash_debug("Chip Erase");
				if (m_StatusRegister & BTFLASH_STATUS_WEL_FLAG)
				{
					m_DelayedCommand = BTFLASH_CMD_CE_C7;
					m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
					return m_LastSignal;
				}
				else
				{
					log(BT8XXEMU_LogError, "Chip Erase requires Write Enable Latch to be set");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
			case BTFLASH_CMD_PP: /* Page Program */
				Flash_debug("Page Program");
				if (m_StatusRegister & BTFLASH_STATUS_WEL_FLAG)
				{
					m_DelayedCommand = BTFLASH_CMD_PP;
					m_RunState = BTFLASH_STATE_PP_ADDR;
					return m_LastSignal;
				}
				else
				{
					log(BT8XXEMU_LogError, "Page Program requires Write Enable Latch to be set");
					m_RunState = BTFLASH_STATE_IGNORE;
					return m_LastSignal;
				}
			case BTFLASH_CMD_CP: /* Continuously Program mode */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_CP)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_DP: /* Deep Power Down */
				Flash_debug("Deep Power Down");
				m_DeepPowerDown = true;
				m_RunState = BTFLASH_STATE_BLANK;
				return m_LastSignal;
			case BTFLASH_CMD_RES: /* Read Electronic ID */
				Flash_debug("Read Electronic ID");
				m_RunState = BTFLASH_STATE_OUT_U8;
				m_NextState = BTFLASH_STATE_OUT_U8;
				startWriteU8(deviceId());
				return m_LastSignal;
			case BTFLASH_CMD_REMS: /* Read Electronic Manufacturer and Device ID */
				Flash_debug("Read Electronic Manufacturer and Device ID");
				m_RunState = BTFLASH_STATE_REMS_ADDR;
				return m_LastSignal;
			case BTFLASH_CMD_REMS2: /* Read ID for 2x IO Mode */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_REMS2)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_REMS4: /* Read ID for 4x IO Mode */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_REMS4)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_REMS4D: /* Read ID for 4x IO DT Mode */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_REMS4D)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_ENSO: /* Enter Secured OTP */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_ENSO)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_EXSO: /* Exit Secured OTP */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_EXSO)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_RDSCUR: /* Read Security Register */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_RDSCUR)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_WRSCUR: /* Write Security Register */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_WRSCUR)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_ESRY: /* Enable SO to output RY/BY# */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_ESRY)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_DSRY: /* Disable SO to output RY/BY# */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_DSRY)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_CLSR: /* Clear SR Fail Flags */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_CLSR)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_HPM: /* High Performance Enable Mode */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_HPM)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_WPSEL: /* Write Protection Selection */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_WPSEL)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_SBLK: /* Single Block Lock */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_SBLK)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_SBULK: /* Single Block Unlock */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_SBULK)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_RDBLOCK: /* Block Protect Read */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_RDBLOCK)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_GBLK: /* Gang Block Lock */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_GBLK)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_GBULK: /* Gang Block Unlock */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_GBULK)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_RSTEN:
				if (m_Sfdp.SwReset)
				{
					Flash_debug("Reset Enable");
					m_DelayedCommand = BTFLASH_CMD_RSTEN;
					m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
				}
				else
				{
					Flash_debug("Reset Enable (No-op)");
					m_RunState = BTFLASH_STATE_IGNORE;
				}
				break;
			case BTFLASH_CMD_RST:
				if (m_Sfdp.SwReset)
				{
					Flash_debug("Reset");
					m_DelayedCommand = BTFLASH_CMD_RST;
					m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
				}
				else
				{
					Flash_debug("Reset (No-op)");
					m_RunState = BTFLASH_STATE_IGNORE;
				}
				break;
			case BTFLASH_CMD_EN4B: /* Enter 4-byte mode */
				Flash_debug("Enter 4-byte mode");
				if (!m_ExtendedAddressing)
				{
					log(BT8XXEMU_LogError, "Can only enter 4-byte mode when extended addressing is available");
				}
				else
				{
					m_DelayedCommand = BTFLASH_CMD_EN4B;
					m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
				}
				return m_LastSignal;
			case BTFLASH_CMD_EX4B: /* Exit 4-byte mode */
				Flash_debug("Exit 4-byte mode");
				if (!m_ExtendedAddressing)
				{
					log(BT8XXEMU_LogError, "Can only exit 4-byte mode when extended addressing is available");
				}
				else
				{
					m_DelayedCommand = BTFLASH_CMD_EX4B;
					m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
				}
				return m_LastSignal;
			case BTFLASH_CMD_4READ_TOP: /* 4x I/O Read Mode (Top) */
				log(BT8XXEMU_LogError, "Flash command not implemented (BTFLASH_CMD_4READ_TOP)");
				m_RunState = BTFLASH_STATE_UNSUPPORTED;
				return m_LastSignal;
			case BTFLASH_CMD_NOOP_EXITQPI:
				Flash_debug("No-op");
				m_RunState = BTFLASH_STATE_IGNORE;
				return m_LastSignal;
			default:
				log(BT8XXEMU_LogError, "Flash command unrecognized (%x)", command);
				break;
			}
		}

		return m_LastSignal;
	}

	uint8_t stateWRSR(uint8_t signal)
	{
		if (readU8Spi1(signal))
		{
			uint8_t data = m_BufferU8;
			m_BufferBits = 0;

			static const uint8_t mask =
				BTFLASH_STATUS_BP0_FLAG
				| BTFLASH_STATUS_BP1_FLAG
				| BTFLASH_STATUS_BP2_FLAG
				| BTFLASH_STATUS_BP3_FLAG
				| BTFLASH_STATUS_QE_FLAG
				| BTFLASH_STATUS_SRWD_FLAG;
			static const uint8_t invMask = (uint8_t)((~mask) & (~BTFLASH_STATUS_WEL_FLAG));
			const uint8_t maskedWrite = data & mask;
			const uint8_t maskedReg = m_StatusRegister & invMask;
			m_StatusRegister = maskedWrite | maskedReg;

			if (data & BTFLASH_STATUS_QE_FLAG)
				writeProtect(false);
			else
				writeProtect(!(signal & BTFLASH_SPI4_WP));

			Flash_debug("WRSR 0x%x %s %s", (int)data,
				(m_StatusRegister & BTFLASH_STATUS_QE_FLAG) ? "QE" : "",
				(m_StatusRegister & BTFLASH_STATUS_SRWD_FLAG) ? "SRWD" : "");

			m_RunState = BTFLASH_STATE_BLANK;
		}

		return m_LastSignal;
	}

	uint8_t stateREMSAddr(uint8_t signal)
	{
		if (readU24Spi1AsU32(signal)) // No 4BYTE
		{
			uint8_t addr = m_BufferU8;
			m_BufferBits = 0;
			if (addr == 0x01) startWrite2U8AsU16(deviceId(), manufacturerId());
			else startWrite2U8AsU16(manufacturerId(), deviceId());
			m_RunState = BTFLASH_STATE_OUT_U16;
			m_NextState = BTFLASH_STATE_OUT_U16;
		}

		return m_LastSignal;
	}

	uint8_t stateRDSFDPAddr(uint8_t signal)
	{
		if (readU24Spi1AsU32SkipLsb(signal, 8)) // No 4BYTE
		{
			uint8_t addr = m_BufferU8;
			m_BufferBits = 0;
			Flash_debug("Read SFDP addr %i", (int)addr);
			m_OutArray = &m_SfdpData[0];
			m_OutArraySize = 256;
			m_OutArrayAt = addr & 0xFF;
			m_OutArrayLoop = true; // VERIFY: Does this loop or not?
			m_RunState = BTFLASH_STATE_OUT_U8_ARRAY;
		}

		return m_LastSignal;
	}

	uint8_t stateREADAddr(uint8_t signal)
	{
		if (readAddressSpi1AsU32SkipLsb(signal, 0))
		{
			size_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("Read READ addr %i", (int)addr);
			m_OutArray = Data;
			m_OutArraySize = Size;
			m_OutArrayAt = addr & (Size - 1);
			m_OutArrayLoop = true;
			m_RunState = BTFLASH_STATE_OUT_U8_ARRAY;
		}

		return m_LastSignal;
	}

	uint8_t stateFAST_READAddr(uint8_t signal)
	{
		static const int dummyCycles[] = { 8, 6, 8, 10 };
		if (readAddressSpi1AsU32SkipLsb(signal,
			dummyCycles[BTFLASH_CONFIGRATION_GET_DC(m_ConfigurationRegister)]))
		{
			size_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("Read FAST_READ addr %i", (int)addr);
			m_OutArray = Data;
			m_OutArraySize = Size;
			m_OutArrayAt = addr & (Size - 1);
			m_OutArrayLoop = true;
			m_RunState = BTFLASH_STATE_OUT_U8_ARRAY;
		}

		return m_LastSignal;
	}

	uint8_t stateFASTDTRDAddr(uint8_t signal)
	{
		if (readAddressSpi1AsU32SkipLsb(signal, 11))
		{
			size_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("Read FASTDTRD addr %i", (int)addr);
			m_OutArray = Data;
			m_OutArraySize = Size;
			m_OutArrayAt = addr & (Size - 1);
			m_OutArrayLoop = true;
			m_RunState = BTFLASH_STATE_OUT_U8_ARRAY_DT;
		}

		return m_LastSignal;
	}

	uint8_t state4READAddr(uint8_t signal)
	{
		if (readAddressSpi4AsU32SkipLsb(signal, 0))
		{
			uint32_t addr = m_BufferU32;
			m_BufferBits = 0;
			// Flash_debug("Read 4READ addr %i", (int)addr);
			m_DelayedCommandAddr = addr;
			if (m_RunState == BTFLASH_STATE_4DTRD_ADDR)
				m_RunState = BTFLASH_STATE_4DTRD_PE;
			else
				m_RunState = BTFLASH_STATE_4READ_PE;
		}

		return m_LastSignal;
	}

	uint8_t state4READPerfEnh(uint8_t signal)
	{
		static const int dummyCycles8[] = { 8, 6, 8, 10 };
		static const int dummyCycles6[] = { 6, 4, 8, 10 };
		const int *dummyCycles = using4ByteAddress() ? dummyCycles8 : dummyCycles6; // VERIFY: Cannot find this in spec, but matches BT815 firmware behaviour
		const bool dtr = m_RunState == BTFLASH_STATE_4DTRD_PE;
		if (readU8Spi4SkipLsb(signal, dtr ? 17 * 4 : // VERIFY: Number of dummy cycles should be 13 (7 dummy cycles), but need to set 17 here for BT817... (BT817 DTR does not work, apparently?)
			(dummyCycles[BTFLASH_CONFIGRATION_GET_DC(m_ConfigurationRegister)] - 2) * 4))
		{
			uint8_t p = m_BufferU8;
			bool pe = ((p & 0xF) == ((~(p >> 4)) & 0xF));
			if (pe)
			{
				if (dtr)
					m_SelectState = BTFLASH_STATE_4DTRD_ADDR;
				else
					m_SelectState = BTFLASH_STATE_4READ_ADDR;
			}
			else m_SelectState = BTFLASH_STATE_COMMAND;
			m_BufferBits = 0;
			Flash_debug("Read %s addr %i, p %x, pe %i",
				dtr ? "4DTRD" : "4READ", 
				(int)m_DelayedCommandAddr, (int)p, (int)pe);
			m_OutArray = Data;
			m_OutArraySize = Size;
			m_OutArrayAt = m_DelayedCommandAddr & (Size - 1);
			m_OutArrayLoop = true;
			m_SignalOutMask = BTFLASH_SPI4_MASK_D4;
			if (dtr)
				m_RunState = BTFLASH_STATE_OUT_U8_ARRAY_DT;
			else
				m_RunState = BTFLASH_STATE_OUT_U8_ARRAY;
		}

		return m_LastSignal;
	}

	uint8_t statePPAddr(uint8_t signal)
	{
		if (readAddressSpi1AsU32SkipLsb(signal, 0))
		{
			uint32_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("PP addr %i", (int)addr);
			m_DelayedCommandAddr = addr;
			m_PageProgramIdx = 0;
			m_PageProgramSize = 0;
			m_RunState = BTFLASH_STATE_PP_READ;
		}

		return m_LastSignal;
	}

	uint8_t statePPRead(uint8_t signal)
	{
		if (readU8Spi1(signal))
		{
			m_PageProgramData[m_PageProgramIdx] = m_BufferU8;
			m_BufferBits = 0;

			++m_PageProgramIdx;
			m_PageProgramIdx &= 0xFF;
			++m_PageProgramSize;
			m_PageProgramSize = std::min(m_PageProgramSize, 256);

			Flash_debug("PP data %x", (int)m_BufferU8);
		}

		return m_LastSignal;
	}

	uint8_t stateSEAddr(uint8_t signal)
	{
		if (readAddressSpi1AsU32SkipLsb(signal, 0))
		{
			uint32_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("SE addr %i", (int)addr);
			m_DelayedCommandAddr = addr;
			m_RunState = BTFLASH_STATE_CS_HIGH_COMMAND;
		}

		return m_LastSignal;
	}

	uint8_t state4PPAddr(uint8_t signal)
	{
		if (readAddressSpi4AsU32SkipLsb(signal, 0))
		{
			uint32_t addr = m_BufferU32;
			m_BufferBits = 0;
			Flash_debug("4PP addr %i", (int)addr);
			m_DelayedCommandAddr = addr;
			m_PageProgramIdx = 0;
			m_PageProgramSize = 0;
			m_RunState = BTFLASH_STATE_4PP_READ;
		}

		return m_LastSignal;
	}

	uint8_t state4PPRead(uint8_t signal)
	{
		if (readU8Spi4(signal))
		{
			m_PageProgramData[m_PageProgramIdx] = m_BufferU8;
			m_BufferBits = 0;

			++m_PageProgramIdx;
			m_PageProgramIdx &= 0xFF;
			++m_PageProgramSize;
			m_PageProgramSize = std::min(m_PageProgramSize, 256);

			Flash_debug("4PP data %x", (int)m_BufferU8);
		}

		return m_LastSignal;
	}

	uint8_t *Data;
	size_t Size;

private:
	bool m_DeepPowerDown;

	HANDLE m_FileHandle;
	HANDLE m_FileMapping;

	bool m_WriteProtect; // WP# low, 0 -> WriteProtect true
	bool m_ResetEnable;

	void(*m_Log)(BT8XXEMU_Flash *sender, void *context, BT8XXEMU_LogType type, const char *message);
	void *m_UserContext;
	bool m_PrintStd;

	uint8_t m_StatusRegister;
	uint8_t m_ConfigurationRegister;
	uint8_t m_ExtendedAddressRegister;

	uint8_t m_DelayedCommand;
	uint32_t m_DelayedCommandAddr;

	uint8_t m_PageProgramData[256];
	int m_PageProgramIdx;
	int m_PageProgramSize;

	union
	{
		FlashSfdp m_Sfdp;
		uint8_t m_SfdpData[256];
	};

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

uint8_t Flash_transferSpi4(Flash *flash, uint8_t signal)
{
	return flash->transferSpi4(signal);
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

	(uint8_t(*)(BT8XXEMU::Flash *, uint8_t))Flash_transferSpi4,

	(uint8_t *(*)(BT8XXEMU::Flash *))Flash_data,
	(size_t(*)(BT8XXEMU::Flash *))Flash_size
};

extern "C" {

BT8XXEMU_EXPORT BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (versionApi != BT8XXEMU_VERSION_API)
	{
		fprintf(stderr, "Incompatible ft8xxemu API version\n");
		return NULL;
	}

	return new ::Flash(params);
}

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* end of file */
