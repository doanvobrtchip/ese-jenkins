/*
BT8XX Emulator Service
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#include <Windows.h>
#include <bt8xxemu.h>

#include <iostream>
#include <vector>

#define BUFSIZE 64 * 1024

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
#define BT8XXEMU_CALL_FLASH_TRANSFER 0x0106
#define BT8XXEMU_CALL_FLASH_CHIP_SELECT 0x0107

int main(int argc, char* argv[])
{
	DWORD nb;
	DWORD len;

	std::vector<BT8XXEMU_Emulator *> emulators;
	emulators.push_back(NULL);

	std::vector<BT8XXEMU_Flash *> flashes;
	flashes.push_back(NULL);

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
					BT8XXEMU_EmulatorMode mode;
					BT8XXEMU_EmulatorParameters params;
				};
				BT8XXEMU_FlashParameters flashParams;
				char str[1024];
				uint8_t data;
				uint8_t chipSelect;
				uint8_t isRunning;
				uint8_t hasInterrupt;
			};
		};
	};
#pragma pack(pop)

#define MESSAGE_SIZE(param) (DWORD)((ptrdiff_t)(void *)(&(param)) + (ptrdiff_t)sizeof(param) - (ptrdiff_t)(void *)(&buffer[0]))
#define STRING_MESSAGE_SIZE() (DWORD)((ptrdiff_t)(void *)(&str[0]) + (ptrdiff_t)strlen(&str[0]) - (ptrdiff_t)(void *)(&buffer[0]))
#define EMULATOR emulators[emulator]
#define FLASH flashes[flash]

	if (argc < 2)
	{
		std::cerr << "Not enough arguments supplied\n";
		return EXIT_FAILURE;
	}
	HANDLE pipe = CreateFileA(
		argv[1], // // "\\\\.\\pipe\\name",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (pipe == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Invalid pipe\n";
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
		return EXIT_FAILURE;
	}
	for (;;)
	{
		if (!ReadFile(pipe, buffer, BUFSIZE, &nb, NULL))
		{
			std::cerr << "Cannot read from pipe\n";
			return EXIT_SUCCESS;
		}
		switch (messageType)
		{
		case BT8XXEMU_CALL_VERSION:
			strcpy(str, BT8XXEMU_version());
			versionApi = BT8XXEMU_VERSION_API;
			len = STRING_MESSAGE_SIZE();
			break;
		case BT8XXEMU_CALL_DEFAULTS:
			BT8XXEMU_defaults(versionApi, &params, mode);
			len = MESSAGE_SIZE(params);
			break;
		case BT8XXEMU_CALL_RUN:
			if (params.Close
				|| params.Graphics
				|| params.Log
				|| params.Main
				|| params.MCUSleep)
			{
				std::cerr << "Callbacks are not permitted in service mode\n";
				return EXIT_FAILURE;
			}
			emulators.push_back(NULL);
			BT8XXEMU_run(versionApi, &emulators[emulators.size() - 1], &params);
			emulator = (uint32_t)emulators.size() - 1;
			len = MESSAGE_SIZE(emulator);
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
			BT8XXEMU_cs(EMULATOR, chipSelect);
			len = MESSAGE_SIZE(emulator);
			break;
		case BT8XXEMU_CALL_HAS_INTERRUPT:
			hasInterrupt = BT8XXEMU_hasInterrupt(EMULATOR);
			len = MESSAGE_SIZE(hasInterrupt);
			break;
		case BT8XXEMU_CALL_FLASH_DEFAULTS:
			BT8XXEMU_Flash_defaults(versionApi, &flashParams);
			len = MESSAGE_SIZE(flashParams);
			break;
		case BT8XXEMU_CALL_FLASH_CREATE:
			flashes.push_back(NULL);
			BT8XXEMU_Flash_create(versionApi, &flashParams);
			flash = (uint32_t)flashes.size() - 1;
			len = MESSAGE_SIZE(flash);
			break;
		case BT8XXEMU_CALL_FLASH_DESTROY:
			BT8XXEMU_Flash_destroy(FLASH);
			FLASH = NULL;
			len = MESSAGE_SIZE(flash);
			break;
		case BT8XXEMU_CALL_FLASH_TRANSFER:
			data = BT8XXEMU_Flash_transfer(FLASH, data);
			len = MESSAGE_SIZE(data);
			break;
		case BT8XXEMU_CALL_FLASH_CHIP_SELECT:
			BT8XXEMU_Flash_chipSelect(FLASH, !!chipSelect);
			len = MESSAGE_SIZE(emulator);
			break;
		}
		if (!WriteFile(pipe, buffer, len, &nb, NULL))
		{
			std::cerr << "Cannot write to pipe\n";
			return EXIT_SUCCESS;
		}
		if (len != nb)
		{
			std::cerr << "Write incomplete\n";
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
