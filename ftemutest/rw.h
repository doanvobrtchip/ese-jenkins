/*
BT8XX Emulator Samples
Copyright (C) 2015  Future Technology Devices International Ltd
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include <bt8xxemu_inttypes.h>

uint8_t rd8(uint32_t address);
uint16_t rd16(uint32_t address);
uint32_t rd32(uint32_t address);
void wr8(uint32_t address, uint8_t value);
void wr16(uint32_t address, uint16_t value);
void wr32(uint32_t address, uint32_t value);

void wrstart(uint32_t address);
void wr8(uint8_t value);
void wr16(uint16_t value);
void wr32(uint32_t value);
int wrstr(const char *str);
void wrend();

/* end of file */
