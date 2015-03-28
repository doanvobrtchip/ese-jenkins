/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

// System includes
#include <stdio.h>
#ifdef WIN32
#	include <windows.h>
#endif

// FT8XXEMU includes
#include <ft8xxemu.h>

// FT900EMU includes
#include <ft900emu_ft32.h>
#include <ft900emu_inttypes.h>
#include <ft900emu_intrin.h>
#include <ft900emu_ft32_def.h>
#include <ft900emu_chip.h>

class FT32SimulatorExit : public FT900EMU::FT32IO
{
public:
	FT32SimulatorExit(FT900EMU::FT32 *ft32) : m_FT32(ft32)
	{
		ft32->io(this);
	}

	~FT32SimulatorExit()
	{
		m_FT32->ioRemove(this);
	}

	// io_a is addr / 4 (read per 4 bytes)
	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be)
	{
		return 0xDEADBEEF;
	}

	virtual void ioWr32(uint32_t io_a, uint32_t io_be, uint32_t io_dout)
	{
		printf(F9EW "<EXITEXIT> (%i)" F9EE, io_dout);
		m_FT32->stop();
	}

	virtual void ioGetRange(uint32_t &from, uint32_t &to)
	{
		from = 0x1FFFCu >> 2;
		to = 0x20000u >> 2;
	}

private:
	FT900EMU::FT32 *m_FT32;

};

class FT800SPISlave : public FT900EMU::SPISlave
{
	virtual void cs(bool cs)
	{
		FT8XXEMU_cs(cs ? 1 : 0);
	}

	virtual uint8_t transfer(uint8_t d)
	{
		return FT8XXEMU_transfer(d);
	}
};

static char *s_InFile = NULL;

static FT900EMU::Chip *s_FT900EMUSystem = NULL;
static FT32SimulatorExit *s_EmulatorExit = NULL;
static FT800SPISlave s_FT800SPISlave;

void mcuSleep(int ms)
{
	s_FT900EMUSystem->ft32()->sleep(ms);
}

void setup()
{
	printf("Setup\n");

#ifdef WIN32
	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(stdIn, &consoleMode);
	SetConsoleMode(stdIn, consoleMode & ~ENABLE_LINE_INPUT & ~ENABLE_WINDOW_INPUT & ~ENABLE_MOUSE_INPUT);
#endif

	// Init system
	s_FT900EMUSystem = new FT900EMU::Chip();
	FT900EMU::FT32 *ft32 = s_FT900EMUSystem->ft32();
	s_EmulatorExit = new FT32SimulatorExit(ft32);
	s_FT900EMUSystem->setSPISlave(0, &s_FT800SPISlave);

	// Load image
	FILE *f = fopen(s_InFile, "rb");
	if (f == NULL)
	{
		printf("DID NOT OPEN FILE\n");
		exit(EXIT_FAILURE);
	}
	int pma = 0;
	for (; ; )
	{
		// printf("WRITE %i\n", pma);
		uint32_t pmd;
		int read;
		if ((read = fread(&pmd, sizeof(pmd), 1, f)) != 1)
			break;
		ft32->pm(pma, pmd);
		++pma;
	}
	fclose(f);
	f = NULL;
}

void loop()
{
	printf("Loop\n");

	s_FT900EMUSystem->ft32()->run();
	FT8XXEMU_stop();

	printf("End\n");
	delete s_EmulatorExit;
	s_EmulatorExit = NULL;
	delete s_FT900EMUSystem;
	s_FT900EMUSystem = NULL;
}

void close()
{
	s_FT900EMUSystem->ft32()->stop();
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage:\n");
		printf("fteve900 [-ft800 | -ft801 (default) | -ft810 | -ft811] <binary>\n");
		return EXIT_FAILURE;
	}

	FT8XXEMU_EmulatorMode mode = FT8XXEMU_EmulatorFT801;

	// Parse command line
	for (int i = 1; i < argc - 1; ++i)
	{
		switch (argv[i][0])
		{
		case '-':
			switch (argv[i][1])
			{
			case 'f':
				switch (argv[i][2])
				{
				case 't':
					switch (argv[i][3])
					{
					case '8':
						mode = (FT8XXEMU_EmulatorMode)atoi(&argv[i][3]);
						break;
					}
					break;
				}
				break;
			}
			break;
		}
	}
	s_InFile = argv[argc - 1];

	FT8XXEMU_EmulatorParameters params;
	FT8XXEMU_defaults(FT8XXEMU_VERSION_API, &params, FT8XXEMU_EmulatorFT801);
	params.MCUSleep = mcuSleep;
	params.Setup = setup;
	params.Loop = loop;
	params.Close = close;
	FT8XXEMU_run(FT8XXEMU_VERSION_API, &params);

	return EXIT_SUCCESS;
}
