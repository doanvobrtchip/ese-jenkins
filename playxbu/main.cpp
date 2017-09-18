/*
BT8XX Emulator Samples
Copyright (C) 2013  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
*/

#define BT815EMU_MODE

#include <Windows.h>

#include <bt8xxemu.h>
#include <stdio.h>
#include <ft800emu_vc.h>

#define TEST_THREAD_REDUCE

class PlayXBU
{
public:
	PlayXBU(const char *file)
	{
		BT8XXEMU_EmulatorParameters params;
		BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params,
#if defined(BT815EMU_MODE)
			BT8XXEMU_EmulatorBT815
#elif defined(FT810EMU_MODE)
			BT8XXEMU_EmulatorFT810
#else
			BT8XXEMU_EmulatorFT801
#endif
		);

		params.Flags =
			BT8XXEMU_EmulatorEnableKeyboard
			| BT8XXEMU_EmulatorEnableMouse
			| BT8XXEMU_EmulatorEnableDebugShortkeys
			| BT8XXEMU_EmulatorEnableCoprocessor
			| BT8XXEMU_EmulatorEnableGraphicsMultithread
			| BT8XXEMU_EmulatorEnableStdOut
			| BT8XXEMU_EmulatorEnableMainPerformance
			;
		params.UserContext = this;

#ifdef TEST_THREAD_REDUCE
		params.ReduceGraphicsThreads = 2;
#endif

		BT8XXEMU_run(BT8XXEMU_VERSION_API, &emulator, &params);

		fh = fopen(file, "rb");
		if (!fh) printf("Failed to open XBU file\n");
		else
		{
			printf("Load XBU '%s'\n", file);

			wr32(REG_HSIZE, 480);
			wr32(REG_VSIZE, 272);
			wr32(REG_PCLK, 5);
		}
	}

	~PlayXBU()
	{
		BT8XXEMU_destroy(emulator);
		emulator = NULL;
	}

	BT8XXEMU_Emulator *emulator;

	void swrbegin(size_t address)
	{
		BT8XXEMU_cs(emulator, 1);

		BT8XXEMU_transfer(emulator, (2 << 6) | ((address >> 16) & 0x3F));
		BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
		BT8XXEMU_transfer(emulator, address & 0xFF);
		// BT8XXEMU_transfer(0x00);
	}

	void swr8(uint8_t value)
	{
		BT8XXEMU_transfer(emulator, value);
	}

	void swr16(uint16_t value)
	{
		BT8XXEMU_transfer(emulator, value & 0xFF);
		BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
	}

	void swr32(uint32_t value)
	{
		BT8XXEMU_transfer(emulator, value & 0xFF);
		BT8XXEMU_transfer(emulator, (value >> 8) & 0xFF);
		BT8XXEMU_transfer(emulator, (value >> 16) & 0xFF);
		BT8XXEMU_transfer(emulator, (value >> 24) & 0xFF);
	}

	void swrend()
	{
		BT8XXEMU_cs(emulator, 0);
	}

	void wr32(size_t address, uint32_t value)
	{
		swrbegin(address);
		swr32(value);
		swrend();
	}

	uint32_t rd32(size_t address)
	{
		BT8XXEMU_cs(emulator, 1);

		BT8XXEMU_transfer(emulator, (address >> 16) & 0x3F);
		BT8XXEMU_transfer(emulator, (address >> 8) & 0xFF);
		BT8XXEMU_transfer(emulator, address & 0xFF);
		BT8XXEMU_transfer(emulator, 0x00);

		uint32_t value;
		value = BT8XXEMU_transfer(emulator, 0);
		value |= BT8XXEMU_transfer(emulator, 0) << 8;
		value |= BT8XXEMU_transfer(emulator, 0) << 16;
		value |= BT8XXEMU_transfer(emulator, 0) << 24;

		BT8XXEMU_cs(emulator, 0);
		return value;
	}

	FILE *fh = NULL;

	int wp = 0;
	int wpr = 0;

	bool okwrite = false;

	int lastround = 0;

	bool loop()
	{
		if (!fh)
		{
			Sleep(10);
			return false;
		}
		else
		{
			int regwp = rd32(REG_CMD_WRITE);
			int rp = rd32(REG_CMD_READ);
			int fullness = ((wp & 0xFFF) - rp) & 0xFFF;
			int freespace = ((4096 - 4) - fullness);

			// printf("rp: %i, wp: %i, wpr: %i, regwp: %i\n", rp, wp, wpr, regwp);
			if (rp == -1)
			{
				printf("rp < 0, error\n");
				printf("Close XBU\n");
				if (fclose(fh)) printf("Error closing vc1dump file\n");
				fh = NULL;
			}
			else
			{
				if (freespace)
					// if (freespace >= 2048)
				{
					int freespacediv = freespace >> 2;

					swrbegin(RAM_CMD + (wp & 0xFFF));
					for (int i = 0; i < freespacediv; ++i)
					{
						uint32_t buffer;
						size_t nb = fread(&buffer, 4, 1, fh);
						if (nb == 1)
						{
							/*if (buffer == CMD_DLSTART) okwrite = true;

							if (okwrite)
							{*/
							swr32(buffer);
							wp += 4;
							//}

							if (buffer == CMD_SWAP)
							{
								wpr = wp;
								swrend();
								wr32(REG_CMD_WRITE, (wpr & 0xFFF));
								swrbegin(RAM_CMD + (wp & 0xFFF));
							}
						}
						else
						{
							printf("Close XBU, nb = %i\n", (int)nb);
							if (fclose(fh)) printf("Error closing vc1dump file\n");
							fh = NULL;
							break;
						}
					}
					swrend();

					if (fh)
					{
						int wprn = (wp - 128);
						if (wprn > wpr)
						{
							wpr = wprn;
							wr32(REG_CMD_WRITE, (wpr & 0xFFF));
						}
					}
					else
					{
						wpr = wp;
						wr32(REG_CMD_WRITE, (wpr & 0xFFF));
					}
				}
				else
				{
					// FT800EMU::System.delay(1000);
				}
				int newround = wpr / 4096;
				if (lastround != newround)
				{
					// printf("new round\n");
					lastround = newround;
				}
			}
		}

		return fh != NULL;
	}
};



// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
#ifdef TEST_THREAD_REDUCE
	SetProcessAffinityMask(GetCurrentProcess(), 3);
#endif
	
	const int nb = 2;
	char *xbu[nb] = {
		"xbu/SCATTER.XBU",
		"xbu/BIRDS.XBU"
	};
	PlayXBU *app[nb];
	for (int i = 0; i < nb; ++i)
		app[i] = new PlayXBU(xbu[i]);
	for (;;)
	{
		bool isRunning = false;
		for (int i = 0; i < nb; ++i)
		{
			app[i]->loop();
			isRunning = isRunning || BT8XXEMU_isRunning(app[i]->emulator);
		}
		if (!isRunning)
			break;
	}
	for (int i = 0; i < nb; ++i)
		delete app[i];
	return 0;
}
