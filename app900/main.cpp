/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include <ft900emu_ft32.h>
#include <stdio.h>
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

	// io_a is addr / 4 (read per 4 bytes)
	virtual uint32_t ioRd32(uint32_t io_a, uint32_t io_be)
	{

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

//static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/helloworld/helloworld.exe";
//static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/kaetest1/kaetest1.exe";
//static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/pi/pi.exe";
//static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/hanoi/hanoi.exe";
//static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/kaetest2/kaetest2.exe";
static const char *infile = "/mnt/fuji/sync/projects_work/ft900emu/ftp/cases/kaetimer1/kaetimer1.exe";

int main(int, char* [])
{
	FT900EMU::Chip *chip = new FT900EMU::Chip();
	FT32SimulatorExit *exit = new FT32SimulatorExit(chip->ft32());
	FILE *f = fopen(infile, "rb");
	if (f == NULL)
	{
		printf("DID NOT OPEN FILE\n");
		return 1;
	}
	int pma = 0;
	for (; ; )
	{
		// printf("WRITE %i\n", pma);
		uint32_t pmd;
		int read;
		if ((read = fread(&pmd, sizeof(pmd), 1, f)) != 1)
			break;
		chip->ft32()->pm(pma, pmd);
		++pma;
	}
	fclose(f);
	f = NULL;
	chip->ft32()->run();

	return 0;
}
