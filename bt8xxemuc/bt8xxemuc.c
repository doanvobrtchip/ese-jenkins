/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

/*
Redistributable file to use the emulator using a separate "bt8xxemus" process.
Should be compiled as part of the EVE HAL with BT8XXEMU_REMOTE enabled in the 
compiler options. When enabled, do not link to the "bt8xxemu" library.
This implements the communication channel with the separated process.
The "bt8xxemus" process provides a graphical debugging user interface.
*/

#include <bt8xxemu.h>

#ifdef BT8XXEMU_REMOTE

#include <stdio.h>

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
#endif

#define BUFSIZE 64 * 1024

#define BT8XXEMU_PIPE_OPEN 0xFF02
#define BT8XXEMU_PIPE_CLOSE 0xFF04
#define BT8XXEMU_PIPE_ECHO 0xFF10

#define BT8XXEMU_CALL_VERSION 0x00
#define BT8XXEMU_CALL_DEFAULTS 0x01
#define BT8XXEMU_CALL_RUN 0x02
#define BT8XXEMU_CALL_STOP 0x03
#define BT8XXEMU_CALL_DESTROY 0x04
#define BT8XXEMU_CALL_IS_RUNNING 0x05
#define BT8XXEMU_CALL_TRANSFER 0x06
#define BT8XXEMU_CALL_CHIP_SELECT 0x07
#define BT8XXEMU_CALL_HAS_INTERRUPT 0x08

#define BT8XXEMU_CALL_FLASH_DEFAULTS 0x0101
#define BT8XXEMU_CALL_FLASH_CREATE 0x0102
#define BT8XXEMU_CALL_FLASH_DESTROY 0x0104
#define BT8XXEMU_CALL_FLASH_TRANSFER_SPI4 0x0109

typedef struct BT8XXEMUC_Remote BT8XXEMUC_Remote;
struct BT8XXEMUC_Remote {
	LONG atomicLock;
	HANDLE pipe;
	union {
		uint32_t flash;
		uint32_t emulator;
	};
};

#pragma pack(push, 1)
typedef union
{
	char buffer[BUFSIZE];
	struct
	{
		uint32_t messageType;
		union
		{
			uint32_t versionApi;
			uint32_t emulator;
			uint32_t flash;
		};
		union
		{
			struct
			{
				BT8XXEMU_EmulatorParameters params;
				BT8XXEMU_EmulatorMode mode;
			};
			BT8XXEMU_FlashParameters flashParams;
			char str[1024];
			uint8_t data;
			uint8_t signal;
			uint8_t chipSelect;
			uint8_t isRunning;
			uint8_t hasInterrupt;
		};
	};
} BT8XXEMUC_Data;
#pragma pack(pop)

#define MESSAGE_SIZE(param) (DWORD)((ptrdiff_t)(void *)(&(data.param)) + (ptrdiff_t)sizeof(data.param) - (ptrdiff_t)(void *)(&data.buffer[0]))
#define STRING_MESSAGE_SIZE() (DWORD)((ptrdiff_t)(void *)(&data.str[0]) + (ptrdiff_t)strlen(&data.str[0]) + 1 - (ptrdiff_t)(void *)(&data.buffer[0]))

static LONG s_AtomicLock = 0;
static LONG s_RefCount = 0;
static HANDLE s_Process = INVALID_HANDLE_VALUE;
static HANDLE s_Pipe = INVALID_HANDLE_VALUE;
static BT8XXEMUC_Data s_VersionData;
static int s_PipeNb = 0;

static void BT8XXEMUC_lockProcessPipe()
{
	while (InterlockedExchange(&s_AtomicLock, 1))
		SwitchToThread();
}

static void BT8XXEMUC_unlockProcessPipe()
{
	InterlockedExchange(&s_AtomicLock, 0);
}

// Open a new process if it has not been opened yet, uses reference counting
static bool BT8XXEMUC_openProcess()
{
	BT8XXEMUC_lockProcessPipe();

	if (!s_RefCount)
	{
		char pipeHandle[MAX_PATH];
		sprintf(pipeHandle, "\\\\.\\pipe\\bt8xxemus_%i", (int)GetCurrentProcessId());

		s_Pipe = CreateNamedPipeA(pipeHandle, PIPE_ACCESS_DUPLEX, 
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			1, BUFSIZE, BUFSIZE, 1000, NULL);

		if (s_Pipe == INVALID_HANDLE_VALUE)
		{
			BT8XXEMUC_unlockProcessPipe();
			return false;
		}

		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		char processCommand[MAX_PATH];
		sprintf(processCommand, "bt8xxemus.exe %s", pipeHandle);
		if (!CreateProcessA(NULL, processCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			CloseHandle(s_Pipe);
			s_Pipe = INVALID_HANDLE_VALUE;
			BT8XXEMUC_unlockProcessPipe();
			return false;
		}

		s_Process = pi.hProcess;

		if (!ConnectNamedPipe(s_Pipe, NULL))
		{
			CloseHandle(s_Pipe);
			s_Pipe = INVALID_HANDLE_VALUE;
			TerminateProcess(s_Process, EXIT_FAILURE);
			s_Process = INVALID_HANDLE_VALUE;
			BT8XXEMUC_unlockProcessPipe();
			return false;
		}
	}
	++s_RefCount;

	BT8XXEMUC_unlockProcessPipe();
	return true;
}

// Reduce the reference count of the process, and closes it when done
static void BT8XXEMUC_closeProcess()
{
	BT8XXEMUC_lockProcessPipe();

	--s_RefCount;
	if (!s_RefCount)
	{
		DWORD nb;
		DWORD len;
		BT8XXEMUC_Data data;

		data.messageType = BT8XXEMU_PIPE_CLOSE;
		len = MESSAGE_SIZE(messageType);

		if (!WriteFile(s_Pipe, data.buffer, len, &nb, NULL) || len != nb)
		{
			CloseHandle(s_Pipe);
			s_Pipe = INVALID_HANDLE_VALUE;
			TerminateProcess(s_Process, EXIT_FAILURE);
			s_Process = INVALID_HANDLE_VALUE;
			BT8XXEMUC_unlockProcessPipe();
			return;
		}

		CloseHandle(s_Pipe);
		s_Pipe = INVALID_HANDLE_VALUE;

		WaitForSingleObject(s_Process, INFINITE);
	}

	BT8XXEMUC_unlockProcessPipe();
}

// Open an additional pipe on the open process. Must be closed
// before closing the process
static HANDLE BT8XXEMUC_openPipe()
{
	BT8XXEMUC_lockProcessPipe();

	++s_PipeNb;

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_PIPE_OPEN;
	data.versionApi = BT8XXEMU_VERSION_API;
	sprintf(data.str, "\\\\.\\pipe\\bt8xxemus_%i_%i", (int)GetCurrentProcessId(), s_PipeNb);
	len = STRING_MESSAGE_SIZE();

	HANDLE pipe = CreateNamedPipeA(data.str, PIPE_ACCESS_DUPLEX, 
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, BUFSIZE, BUFSIZE, 1000, NULL);

	if (pipe == INVALID_HANDLE_VALUE
		|| !WriteFile(s_Pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(s_Pipe, data.buffer, BUFSIZE, &nb, NULL)
		|| !ConnectNamedPipe(pipe, NULL))
	{
		BT8XXEMUC_unlockProcessPipe();
		return INVALID_HANDLE_VALUE;
	}

	BT8XXEMUC_unlockProcessPipe();
	return pipe;
}

static void BT8XXEMUC_lockPipe(BT8XXEMUC_Remote *emulator)
{
	while (InterlockedExchange(&emulator->atomicLock, 1))
		SwitchToThread();
}

static void BT8XXEMUC_unlockPipe(BT8XXEMUC_Remote *emulator)
{
	InterlockedExchange(&emulator->atomicLock, 0);
}

// Close a pipe
static void BT8XXEMUC_closePipe(BT8XXEMUC_Remote *emulator)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_PIPE_CLOSE;
	len = MESSAGE_SIZE(messageType);
	WriteFile(emulator->pipe, data.buffer, len, &nb, NULL);
	CloseHandle(emulator->pipe);
	emulator->pipe = INVALID_HANDLE_VALUE;

	BT8XXEMUC_unlockPipe(emulator);
}

const char *BT8XXEMU_version()
{
	if (BT8XXEMUC_openProcess())
	{
		BT8XXEMUC_lockProcessPipe();

		DWORD nb;
		DWORD len;

		s_VersionData.messageType = BT8XXEMU_CALL_VERSION;
		s_VersionData.versionApi = BT8XXEMU_VERSION_API;

		; {
			BT8XXEMUC_Data data;
			len = MESSAGE_SIZE(versionApi);
		}

		if (!WriteFile(s_Pipe, s_VersionData.buffer, len, &nb, NULL) || len != nb
			|| !ReadFile(s_Pipe, s_VersionData.buffer, BUFSIZE, &nb, NULL))
		{
			BT8XXEMUC_unlockProcessPipe();
			BT8XXEMUC_closeProcess();
			return "BT8XX Emulator Library\nNot Responding";
		}

		BT8XXEMUC_unlockProcessPipe();
		BT8XXEMUC_closeProcess();

		return s_VersionData.str;
	}

	return "BT8XX Emulator Library\nNot Installed";
}

void BT8XXEMU_defaults(uint32_t versionApi, BT8XXEMU_EmulatorParameters *params, BT8XXEMU_EmulatorMode mode)
{
	if (BT8XXEMUC_openProcess())
	{
		BT8XXEMUC_lockProcessPipe();

		DWORD nb;
		DWORD len;
		BT8XXEMUC_Data data;

		data.messageType = BT8XXEMU_CALL_DEFAULTS;
		data.versionApi = BT8XXEMU_VERSION_API;
		memcpy(&data.params, params, sizeof(BT8XXEMU_EmulatorParameters));
		data.mode = mode;
		len = MESSAGE_SIZE(mode);

		if (!WriteFile(s_Pipe, data.buffer, len, &nb, NULL) || len != nb
			|| !ReadFile(s_Pipe, data.buffer, BUFSIZE, &nb, NULL))
		{
			BT8XXEMUC_unlockProcessPipe();
			BT8XXEMUC_closeProcess();
			memset(params, 0, sizeof(BT8XXEMU_EmulatorParameters));
			return;
		}

		BT8XXEMUC_unlockProcessPipe();
		BT8XXEMUC_closeProcess();

		memcpy(params, &data.params, sizeof(BT8XXEMU_EmulatorParameters));
		return;
	}

	memset(params, 0, sizeof(BT8XXEMU_EmulatorParameters));
}

void BT8XXEMU_run(uint32_t versionApi, BT8XXEMU_Emulator **emulator, const BT8XXEMU_EmulatorParameters *params)
{
	if (BT8XXEMUC_openProcess())
	{
		*emulator = malloc(sizeof(BT8XXEMU_Emulator));
		memset(*emulator, 0, sizeof(BT8XXEMU_Emulator));
		(*emulator)->pipe = BT8XXEMUC_openPipe(); // Create a separate pipe for each emulator instance

		if ((*emulator)->pipe == INVALID_HANDLE_VALUE)
		{
			BT8XXEMUC_closeProcess();
			free(*emulator);
			*emulator = NULL;
			return;
		}

		DWORD nb;
		DWORD len;
		BT8XXEMUC_Data data;

		data.messageType = BT8XXEMU_CALL_RUN;
		data.versionApi = BT8XXEMU_VERSION_API;
		memcpy(&data.params, params, sizeof(BT8XXEMU_EmulatorParameters));
		data.params.Flash = (void *)(ptrdiff_t)data.params.Flash->flash;
		len = MESSAGE_SIZE(params);

		data.params.Main = NULL;
		data.params.Close = NULL; // Temporary

		if (!WriteFile((*emulator)->pipe, data.buffer, len, &nb, NULL) || len != nb
			|| !ReadFile((*emulator)->pipe, data.buffer, BUFSIZE, &nb, NULL))
		{
			BT8XXEMUC_closePipe(*emulator);
			BT8XXEMUC_closeProcess();
			free(*emulator);
			*emulator = NULL;
			return;
		}

		(*emulator)->emulator = data.emulator;

		if (params->Main)
		{
			params->Main((*emulator), params->UserContext);
		}

		return;
	}
	
	*emulator = NULL;
}

void BT8XXEMU_stop(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_STOP;
	data.emulator = emulator->emulator;
	len = MESSAGE_SIZE(emulator);
	
	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		// ...
	}

	BT8XXEMUC_unlockPipe(emulator);
}

void BT8XXEMU_destroy(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_DESTROY;
	data.emulator = emulator->emulator;
	len = MESSAGE_SIZE(emulator);

	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		// ...
	}

	BT8XXEMUC_unlockPipe(emulator);

	BT8XXEMUC_closePipe(emulator);
	BT8XXEMUC_closeProcess();
	free(emulator);
}

int BT8XXEMU_isRunning(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_IS_RUNNING;
	data.emulator = emulator->emulator;
	len = MESSAGE_SIZE(emulator);

	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		BT8XXEMUC_unlockPipe(emulator);
		return 0;
	}

	BT8XXEMUC_unlockPipe(emulator);
	return data.isRunning;
}

uint8_t BT8XXEMU_transfer(BT8XXEMU_Emulator *emulator, uint8_t value)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_TRANSFER;
	data.emulator = emulator->emulator;
	data.data = value;
	len = MESSAGE_SIZE(data);

	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		BT8XXEMUC_unlockPipe(emulator);
		return 0;
	}

	BT8XXEMUC_unlockPipe(emulator);
	return data.data;
}

void BT8XXEMU_chipSelect(BT8XXEMU_Emulator *emulator, int cs)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_CHIP_SELECT;
	data.emulator = emulator->emulator;
	data.chipSelect = cs;
	len = MESSAGE_SIZE(chipSelect);

	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		// ...
	}

	BT8XXEMUC_unlockPipe(emulator);
}

int BT8XXEMU_hasInterrupt(BT8XXEMU_Emulator *emulator)
{
	BT8XXEMUC_lockPipe(emulator);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_HAS_INTERRUPT;
	data.emulator = emulator->emulator;
	len = MESSAGE_SIZE(emulator);

	if (!WriteFile(emulator->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(emulator->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		BT8XXEMUC_unlockPipe(emulator);
		return 0;
	}

	BT8XXEMUC_unlockPipe(emulator);
	return data.hasInterrupt;
}

void BT8XXEMU_Flash_defaults(uint32_t versionApi, BT8XXEMU_FlashParameters *params)
{
	if (BT8XXEMUC_openProcess())
	{
		BT8XXEMUC_lockProcessPipe();

		DWORD nb;
		DWORD len;
		BT8XXEMUC_Data data;

		data.messageType = BT8XXEMU_CALL_FLASH_DEFAULTS;
		data.versionApi = BT8XXEMU_VERSION_API;
		memcpy(&data.flashParams, params, sizeof(BT8XXEMU_FlashParameters));
		len = MESSAGE_SIZE(flashParams);

		if (!WriteFile(s_Pipe, data.buffer, len, &nb, NULL) || len != nb
			|| !ReadFile(s_Pipe, data.buffer, BUFSIZE, &nb, NULL))
		{
			BT8XXEMUC_unlockProcessPipe();
			BT8XXEMUC_closeProcess();
			memset(params, 0, sizeof(BT8XXEMU_FlashParameters));
			return;
		}

		BT8XXEMUC_unlockProcessPipe();
		BT8XXEMUC_closeProcess();

		memcpy(params, &data.flashParams, sizeof(BT8XXEMU_FlashParameters));
		return;
	}

	memset(params, 0, sizeof(BT8XXEMU_FlashParameters));
}

BT8XXEMU_Flash *BT8XXEMU_Flash_create(uint32_t versionApi, const BT8XXEMU_FlashParameters *params)
{
	if (BT8XXEMUC_openProcess())
	{
		BT8XXEMU_Flash *flash = malloc(sizeof(BT8XXEMU_Flash));
		memset(flash, 0, sizeof(BT8XXEMU_Flash));
		flash->pipe = BT8XXEMUC_openPipe(); // Create a separate pipe for each emulator instance

		if (flash->pipe == INVALID_HANDLE_VALUE)
		{
			BT8XXEMUC_closeProcess();
			free(flash);
			return NULL;
		}

		DWORD nb;
		DWORD len;
		BT8XXEMUC_Data data;

		data.messageType = BT8XXEMU_CALL_FLASH_CREATE;
		data.versionApi = BT8XXEMU_VERSION_API;
		memcpy(&data.flashParams, params, sizeof(BT8XXEMU_FlashParameters));
		len = MESSAGE_SIZE(flashParams);

		if (!WriteFile(flash->pipe, data.buffer, len, &nb, NULL) || len != nb
			|| !ReadFile(flash->pipe, data.buffer, BUFSIZE, &nb, NULL))
		{
			BT8XXEMUC_closePipe(flash);
			BT8XXEMUC_closeProcess();
			free(flash);
			return NULL;
		}

		flash->flash = data.flash;

		return flash;
	}

	return NULL;
}

void BT8XXEMU_Flash_destroy(BT8XXEMU_Flash *flash)
{
	BT8XXEMUC_lockPipe(flash);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_FLASH_DESTROY;
	data.flash = flash->flash;
	len = MESSAGE_SIZE(emulator);

	if (!WriteFile(flash->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(flash->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		// ...
	}

	BT8XXEMUC_unlockPipe(flash);

	BT8XXEMUC_closePipe(flash);
	BT8XXEMUC_closeProcess();
	free(flash);
}

uint8_t BT8XXEMU_Flash_transferSpi4(BT8XXEMU_Flash *flash, uint8_t signal)
{
	BT8XXEMUC_lockPipe(flash);

	DWORD nb;
	DWORD len;
	BT8XXEMUC_Data data;

	data.messageType = BT8XXEMU_CALL_FLASH_TRANSFER_SPI4;
	data.flash = flash->flash;
	data.signal = signal;
	len = MESSAGE_SIZE(signal);

	if (!WriteFile(flash->pipe, data.buffer, len, &nb, NULL) || len != nb
		|| !ReadFile(flash->pipe, data.buffer, BUFSIZE, &nb, NULL))
	{
		BT8XXEMUC_unlockPipe(flash);
		return 0;
	}

	BT8XXEMUC_unlockPipe(flash);
	return data.data;
}

#endif

/* end of file */