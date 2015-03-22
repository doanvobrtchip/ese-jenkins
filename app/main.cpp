/**
 * main.cpp
 * $Id$
 * \file main.cpp
 * \brief main.cpp
 * \date 2013-06-21 21:51GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include <ft8xxemu.h>
#include <wiring.h>
#include <stdio.h>
#include <string.h>

void setup();
void loop();

bool graphics(bool output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags)
{
	static int i = 0;
	printf("frame %i\n", i);
	++i;
	return (i < 100);
}

extern void writeMemory(int index, int value);
extern void writeMemoryPtrInit();
extern void writeMemoryPtr(int index, int value);
class MemoryWriter
{
public:
	void init();
	void write(int index, int value);
	int *m_Memory;
};
class MemoryWriterDirect
{
public:
	void init();
	void write(int index, int value);
	int m_MemoryTest[64 * 1024];
	int m_Memory[64 * 1024];
};
class MemoryWriterStatic
{
public:
	static void writeMemory(int index, int value);
	static void writeMemoryPtr(int index, int value);
};

// int __stdcall WinMain(void *, void *, void *, int)
int main(int, char* [])
{
#if 0
	printf("Test performance\n");

	FT800EMU::System.begin();
	MemoryWriter *mw = new MemoryWriter();
	MemoryWriterDirect *md = new MemoryWriterDirect();
	mw->init();
	md->init();
	writeMemoryPtrInit();
	for (int j = 0; j < 4; ++j)
	{
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				writeMemory(i, r);
			}
			int end = micros();
			printf("STAT ARR: %i\n", (end - start));
		}
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				MemoryWriterStatic::writeMemory(i, r);
			}
			int end = micros();
			printf("SCLS ARR: %i\n", (end - start));
		}
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				md->write(i, r);
			}
			int end = micros();
			printf("CLAS ARR: %i\n", (end - start));
		}
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				writeMemoryPtr(i, r);
			}
			int end = micros();
			printf("STAT PTR: %i\n", (end - start));
		}
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				MemoryWriterStatic::writeMemoryPtr(i, r);
			}
			int end = micros();
			printf("SCLS PTR: %i\n", (end - start));
		}
		{
			int start = micros();
			for (int r = 0; r < 16 * 1024; ++r)
			for (int i = 0; i < (64 * 1024); ++i)
			{
				mw->write(i, r);
			}
			int end = micros();
			printf("CLAS PTR: %i\n", (end - start));
		}
	}
	FT800EMU::System.end();
	return 0;
#endif

	/* VS2008
STAT ARR: 4321467
SCLS ARR: 4206136
CLAS ARR: 2386122
STAT PTR: 2444801
SCLS PTR: 2447703
CLAS PTR: 3056526
STAT ARR: 4202668
SCLS ARR: 4205797
CLAS ARR: 2382344
STAT PTR: 2449285
SCLS PTR: 2444894
CLAS PTR: 3039123
STAT ARR: 4202549
SCLS ARR: 4203440
CLAS ARR: 2388524
STAT PTR: 2444644
SCLS PTR: 2456331
CLAS PTR: 3097185
STAT ARR: 4201627
SCLS ARR: 4201960
CLAS ARR: 2384131
STAT PTR: 2442732
SCLS PTR: 2451901
CLAS PTR: 2965520
	*/

	/* GCC 4.7
STAT ARR: 1606234
SCLS ARR: 1858859
CLAS ARR: 1590866
STAT PTR: 1613261
SCLS PTR: 1870076
CLAS PTR: 1608162
STAT ARR: 1597123
SCLS ARR: 1858293
CLAS ARR: 1589914
STAT PTR: 1613796
SCLS PTR: 1869922
CLAS PTR: 1605504
STAT ARR: 1596582
SCLS ARR: 1858737
CLAS ARR: 1590617
STAT PTR: 1613626
SCLS PTR: 1868997
CLAS PTR: 1607147
STAT ARR: 1592557
SCLS ARR: 1860342
CLAS ARR: 1589700
STAT PTR: 1611117
SCLS PTR: 1869853
CLAS PTR: 1606077
	*/
	
/* MinGW, GCC 4.8 (on old intel pc)

STAT ARR: 3224030
SCLS ARR: 3811983
CLAS ARR: 3763551
STAT PTR: 3185080
SCLS PTR: 3691128
CLAS PTR: 3737705
STAT ARR: 3288824
SCLS ARR: 3787269
CLAS ARR: 3808468
STAT PTR: 3293231
SCLS PTR: 3739222
CLAS PTR: 3736708
STAT ARR: 3204155
SCLS ARR: 3743776
CLAS ARR: 3811205
STAT PTR: 3198485
SCLS PTR: 3670267
CLAS PTR: 3709244
STAT ARR: 3197069
SCLS ARR: 3670050
CLAS ARR: 3623427
STAT PTR: 3157212
SCLS PTR: 3697523
CLAS PTR: 3693334

*/

	FT800EMU::ARDUINO::setCSPin(9);
	FT8XXEMU_EmulatorParameters params;
	FT8XXEMU_defaults(FT8XXEMU_VERSION_API, &params, FT8XXEMU_EmulatorFT810);
	params.Setup = setup;
	params.Loop = loop;
	// params.Graphics = graphics;
	params.Mode = FT8XXEMU_EmulatorFT810;
	FT8XXEMU_run(FT8XXEMU_VERSION_API, &params);
	return 0;
}
