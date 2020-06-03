/*
BT8XX Emulator Service
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <bt8xxemu.h>

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

#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

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

#ifndef _M_X64
#define BT8XXEMUS_32BIT
#endif

volatile bool s_Running = true;
std::mutex s_Mutex;

std::vector<BT8XXEMU_Emulator *> s_Emulators;
std::vector<BT8XXEMU_Flash *> s_Flashes;
#ifdef BT8XXEMUS_32BIT
std::vector<uint64_t> s_UserContexts;
std::map<uint64_t, void *> s_UserContextMap;
#endif

std::vector<HANDLE> s_Threads;
std::vector<std::string> s_ThreadPipes;

DWORD WINAPI runPipe(const char *pipeName)
{
	DWORD nb;
	DWORD len;

#pragma pack(push, 1)
	union
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
					// BT8XXEMU_EmulatorParameters params;
					struct
					{
						int32_t flags;
						BT8XXEMU_EmulatorMode mode;
						uint32_t mousePressure;
						uint32_t externalFrequency;
						uint32_t reduceGraphicsThreads;
						wchar_t romFilePath[260];
						wchar_t otpFilePath[260];
						wchar_t coprocessorRomFilePath[260];
						int64_t userContext;
						int32_t flash;
					} remoteParams;
					BT8XXEMU_EmulatorMode mode;
				};
				// BT8XXEMU_FlashParameters flashParams;
				struct
				{
					wchar_t deviceType[26];
					uint64_t sizeBytes;
					wchar_t dataFilePath[260];
					wchar_t statusFilePath[260];
					bool persistent;
					bool stdOut;
					int64_t userContext;
				} remoteFlashParams;
				char str[1024];
				uint8_t data;
				uint8_t signal;
				uint8_t chipSelect;
				uint8_t isRunning;
				uint8_t hasInterrupt;
			};
		};
	};
#pragma pack(pop)

	BT8XXEMU_EmulatorParameters params = { 0 };
	BT8XXEMU_FlashParameters flashParams = { 0 };

	std::map<uint64_t, void *>::iterator userContext;

#define MESSAGE_SIZE(param) (DWORD)((ptrdiff_t)(void *)(&(param)) + (ptrdiff_t)sizeof(param) - (ptrdiff_t)(void *)(&buffer[0]))
#define STRING_MESSAGE_SIZE() (DWORD)((ptrdiff_t)(void *)(&str[0]) + (ptrdiff_t)strlen(&str[0]) + 1 - (ptrdiff_t)(void *)(&buffer[0]))
#define EMULATOR s_Emulators[emulator]
#define FLASH s_Flashes[flash]

	HANDLE pipe = CreateFileA(
		pipeName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (pipe == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Invalid pipe '" << pipeName << "'\n";
		return EXIT_FAILURE;
	}
	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	if (!SetNamedPipeHandleState(
		pipe,    // pipe handle 
		&pipeMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL))// don't set maximum time 
	{
		std::cerr << "Unable to set message mode\n";
		CloseHandle(pipe);
		return EXIT_FAILURE;
	}
	for (;;)
	{
		if (!ReadFile(pipe, buffer, BUFSIZE, &nb, NULL))
		{
			std::cerr << "Cannot read from pipe\n";
			CloseHandle(pipe);
			return EXIT_SUCCESS;
		}
		switch (messageType)
		{
		case BT8XXEMU_PIPE_OPEN:
			s_Mutex.lock();
			if (!s_Running)
			{
				s_Mutex.unlock();
				std::cerr << "Cannot create during exit\n";
				CloseHandle(pipe);
				return EXIT_FAILURE;
			}
			s_ThreadPipes.push_back(std::string(str));
			s_Threads.push_back(CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)runPipe,
				(LPVOID)&s_ThreadPipes[s_ThreadPipes.size() - 1][0], 
				0, NULL));
			if (!s_Threads[s_Threads.size() - 1])
			{
				s_Mutex.unlock();
				std::cerr << "Cannot create pipe thread\n";
				CloseHandle(pipe);
				return EXIT_FAILURE;
			}
			len = MESSAGE_SIZE(messageType);
			s_Mutex.unlock();
			break;
		case BT8XXEMU_PIPE_CLOSE:
			CloseHandle(pipe);
			return EXIT_SUCCESS;
		case BT8XXEMU_PIPE_ECHO:
			std::cout << str << std::endl;
			len = MESSAGE_SIZE(messageType);
			break;
		case BT8XXEMU_CALL_VERSION:
			strcpy(str, BT8XXEMU_version());
			versionApi = BT8XXEMU_VERSION_API;
			len = STRING_MESSAGE_SIZE();
			break;
		case BT8XXEMU_CALL_DEFAULTS:
			BT8XXEMU_defaults(versionApi, &params, mode);
			remoteParams.flags = params.Flags;
			remoteParams.mode = params.Mode;
			remoteParams.mousePressure = params.MousePressure;
			remoteParams.reduceGraphicsThreads = params.ReduceGraphicsThreads;
			wcscpy(remoteParams.romFilePath, params.RomFilePath);
			wcscpy(remoteParams.otpFilePath, params.OtpFilePath);
			wcscpy(remoteParams.coprocessorRomFilePath, params.CoprocessorRomFilePath);
			assert(params.UserContext == NULL);
#ifdef BT8XXEMUS_32BIT
			remoteParams.userContext = s_UserContexts[(intptr_t)params.UserContext];
#else
			remoteParams.userContext = (uint64_t)params.UserContext;
#endif
			assert(params.Flash == NULL);
			remoteParams.flash = 0;
			len = MESSAGE_SIZE(remoteParams);
			break;
		case BT8XXEMU_CALL_RUN:
			params.Flags = remoteParams.flags;
			params.Mode = remoteParams.mode;
			params.MousePressure = remoteParams.mousePressure;
			params.ReduceGraphicsThreads = remoteParams.reduceGraphicsThreads;
			wcscpy(params.RomFilePath, remoteParams.romFilePath);
			wcscpy(params.OtpFilePath, remoteParams.otpFilePath);
			wcscpy(params.CoprocessorRomFilePath, remoteParams.coprocessorRomFilePath);
			// params.UserContext = ...;
			params.Flash = s_Flashes[remoteParams.flash];
			s_Mutex.lock();
#ifdef BT8XXEMUS_32BIT
			userContext = s_UserContextMap.find(remoteParams.userContext);
			if (userContext != s_UserContextMap.end())
			{
				params.UserContext = userContext->second;
				assert(s_UserContexts[(intptr_t)params.UserContext] == remoteParams.userContext);
			}
			else
			{
				s_UserContexts.push_back(remoteParams.userContext);
				params.UserContext = (void *)(s_UserContexts.size() - 1);
				s_UserContextMap[(intptr_t)params.UserContext] = params.UserContext;
			}
#else
			params.UserContext = (void *)remoteParams.userContext;
#endif
			s_Emulators.push_back(NULL);
			BT8XXEMU_run(versionApi, &s_Emulators[s_Emulators.size() - 1], &params);
			emulator = (uint32_t)s_Emulators.size() - 1;
			len = MESSAGE_SIZE(emulator);
			s_Mutex.unlock();
			break;
		case BT8XXEMU_CALL_STOP:
			BT8XXEMU_stop(EMULATOR);
			len = MESSAGE_SIZE(emulator);
			break;
		case BT8XXEMU_CALL_DESTROY:
			BT8XXEMU_destroy(EMULATOR);
			EMULATOR = NULL;
			len = MESSAGE_SIZE(emulator);
			break;
		case BT8XXEMU_CALL_IS_RUNNING:
			isRunning = BT8XXEMU_isRunning(EMULATOR);
			len = MESSAGE_SIZE(isRunning);
			break;
		case BT8XXEMU_CALL_TRANSFER:
			data = BT8XXEMU_transfer(EMULATOR, data);
			len = MESSAGE_SIZE(data);
			break;
		case BT8XXEMU_CALL_CHIP_SELECT:
			BT8XXEMU_chipSelect(EMULATOR, chipSelect);
			len = MESSAGE_SIZE(emulator);
			break;
		case BT8XXEMU_CALL_HAS_INTERRUPT:
			hasInterrupt = BT8XXEMU_hasInterrupt(EMULATOR);
			len = MESSAGE_SIZE(hasInterrupt);
			break;
		case BT8XXEMU_CALL_FLASH_DEFAULTS:
			BT8XXEMU_Flash_defaults(versionApi, &flashParams);
			wcscpy(remoteFlashParams.deviceType, flashParams.DeviceType);
			remoteFlashParams.sizeBytes = flashParams.SizeBytes;
			wcscpy(remoteFlashParams.dataFilePath, flashParams.DataFilePath);
			wcscpy(remoteFlashParams.statusFilePath, flashParams.StatusFilePath);
			remoteFlashParams.persistent = flashParams.Persistent;
			remoteFlashParams.stdOut = flashParams.StdOut;
			assert(flashParams.UserContext == NULL);
#ifdef BT8XXEMUS_32BIT
			remoteFlashParams.userContext = s_UserContexts[(intptr_t)flashParams.UserContext];
#else
			remoteFlashParams.userContext = (int64_t)flashParams.UserContext;
#endif
			len = MESSAGE_SIZE(remoteFlashParams);
			break;
		case BT8XXEMU_CALL_FLASH_CREATE:
			s_Mutex.lock();
			s_Flashes.push_back(NULL);
			wcscpy(flashParams.DeviceType, remoteFlashParams.deviceType);
			flashParams.SizeBytes = remoteFlashParams.sizeBytes;
			wcscpy(flashParams.DataFilePath, remoteFlashParams.dataFilePath);
			wcscpy(flashParams.StatusFilePath, remoteFlashParams.statusFilePath);
			flashParams.Persistent = remoteFlashParams.persistent;
			flashParams.StdOut = remoteFlashParams.stdOut;
#ifdef BT8XXEMUS_32BIT
			userContext = s_UserContextMap.find(remoteFlashParams.userContext);
			if (userContext != s_UserContextMap.end())
			{
				flashParams.UserContext = userContext->second;
				assert(s_UserContexts[(intptr_t)flashParams.UserContext] == remoteFlashParams.userContext);
			}
			else
			{
				s_UserContexts.push_back(remoteFlashParams.userContext);
				flashParams.UserContext = (void *)(s_UserContexts.size() - 1);
				s_UserContextMap[(intptr_t)flashParams.UserContext] = flashParams.UserContext;
			}
#else
			flashParams.UserContext = (void *)remoteFlashParams.userContext;
#endif
			s_Flashes[s_Flashes.size() - 1] = BT8XXEMU_Flash_create(versionApi, &flashParams);
			flash = (uint32_t)s_Flashes.size() - 1;
			len = MESSAGE_SIZE(flash);
			s_Mutex.unlock();
			break;
		case BT8XXEMU_CALL_FLASH_DESTROY:
			BT8XXEMU_Flash_destroy(FLASH);
			FLASH = NULL;
			len = MESSAGE_SIZE(flash);
			break;
		case BT8XXEMU_CALL_FLASH_TRANSFER_SPI4:
			signal = BT8XXEMU_Flash_transferSpi4(FLASH, signal);
			len = MESSAGE_SIZE(signal);
			break;
		default:
			std::cerr << "Unknown message\n";
			CloseHandle(pipe);
			return EXIT_FAILURE;
		}
		if (!WriteFile(pipe, buffer, len, &nb, NULL))
		{
			std::cerr << "Cannot write to pipe\n";
			CloseHandle(pipe);
			return EXIT_SUCCESS;
		}
		if (len != nb)
		{
			std::cerr << "Write incomplete\n";
			CloseHandle(pipe);
			return EXIT_FAILURE;
		}
	}
	CloseHandle(pipe);
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Not enough arguments supplied\n";
		return EXIT_FAILURE;
	}

	s_Emulators.push_back(NULL);
	s_Flashes.push_back(NULL);

#ifdef BT8XXEMUS_32BIT
	s_UserContexts.push_back(0);
	s_UserContextMap[0] = NULL;
#endif

	s_Threads.push_back(NULL);
	s_ThreadPipes.push_back(std::string());

	int res = runPipe(argv[1]);

	s_Mutex.lock();
	s_Running = false;
	s_Mutex.unlock();

	WaitForMultipleObjects(
		(DWORD)s_Threads.size(),
		&s_Threads[0], 
		TRUE, INFINITE);

	for (size_t i = 0; i < s_Threads.size(); ++i)
		CloseHandle(s_Threads[i]);

	for (BT8XXEMU_Emulator *emulator : s_Emulators)
		if (emulator) BT8XXEMU_destroy(emulator);
	s_Emulators.clear();

	for (BT8XXEMU_Flash *flash : s_Flashes)
		if (flash) BT8XXEMU_Flash_destroy(flash);
	s_Flashes.clear();

	return res;
}

/* end of file */
