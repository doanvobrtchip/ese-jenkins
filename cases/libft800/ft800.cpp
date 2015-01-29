#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libft900.h"

#define FT800_SEL_PIN   FT900_SPI_SS0

static void scu(uint8_t a, uint8_t b, uint8_t c)
{
  ft900_spim_sel(FT800_SEL_PIN);
  ft900_spim_transfer(a);
  ft900_spim_transfer(b);
  ft900_spim_transfer(c);
  ft900_spim_unsel();
  usleep(100000);
}

#include "vc.h"

#include "ft800.h"

VCClass VC;

void VCClass::begin()
{
  ft900_spim_activate();
  ft900_spim_unsel();
  wp = 0;
  freespace = 4096 - 4;

  scu(0, 0, 0);
  scu(0x44, 0, 0);
  scu(0x68, 0, 0);
}

#define highByte(a) ((a) >> 8)
#define lowByte(a) ((a) & 0xff)

void VCClass::__start(uint32_t addr) // start an SPI transaction to addr
{
  ft900_spim_sel(FT800_SEL_PIN);
  ft900_spim_transfer(addr >> 16);
  ft900_spim_transfer(highByte(addr));
  ft900_spim_transfer(lowByte(addr));  
}

void VCClass::__wstart(uint32_t addr) // start an SPI write transaction to addr
{
  ft900_spim_sel(FT800_SEL_PIN);
  ft900_spim_transfer(0x80 | (addr >> 16));
  ft900_spim_transfer(highByte(addr));
  ft900_spim_transfer(lowByte(addr));  
}

void VCClass::__end() // end the SPI transaction
{
  ft900_spim_unsel();
}

uint8_t VCClass::rd(uint32_t addr)
{
  __start(addr);
  ft900_spim_transfer(0);  // dummy
  uint8_t r = ft900_spim_transfer(0);
  __end();
  return r;
}

void VCClass::wr(uint32_t addr, uint8_t v)
{
  __wstart(addr);
  ft900_spim_transfer(v);
  __end();
}

unsigned int VCClass::rd16(uint32_t addr)
{
  unsigned int r;
  __start(addr);
  ft900_spim_transfer(0);  // dummy
  r = ft900_spim_transfer(0);
  r |= (ft900_spim_transfer(0) << 8);
  __end();
  return r;
}

uint32_t VCClass::rd32(uint32_t addr)
{
  unsigned int r;
  __start(addr);
  ft900_spim_transfer(0);  // dummy
  r = ft900_spim_transfer(0);
  r |= (ft900_spim_transfer(0) << 8);
  r |= (ft900_spim_transfer(0) << 16);
  r |= (ft900_spim_transfer(0) << 24);
  __end();
  return r;
}

void VCClass::wr16(uint32_t addr, unsigned int v)
{
  __wstart(addr);
  ft900_spim_transfer(lowByte(v));
  ft900_spim_transfer(highByte(v));
  __end();
}

void VCClass::wr32(uint32_t addr, unsigned long v)
{
  __wstart(addr);
  ft900_spim_transfer(v);
  ft900_spim_transfer(v >> 8);
  ft900_spim_transfer(v >> 16);
  ft900_spim_transfer(v >> 24);
  __end();
}

void VCClass::command(uint32_t c)
{
  wpstart(4);
  spi32(c);
  __end();
  wpbump(4);
}

static void spi16(int a)
{
  ft900_spim_transfer(lowByte(a));
  ft900_spim_transfer(highByte(a));
}

void VCClass::spi32(uint32_t a)
{
  ft900_spim_transfer(a);
  ft900_spim_transfer(a >> 8);
  ft900_spim_transfer(a >> 16);
  ft900_spim_transfer(a >> 24);
}

static void spis(const char *s)
{
  do {
    ft900_spim_transfer(*s);
  } while (*s++);
}

#include "vccmds.h"

void VCClass::waitidle()
{
  while (wp != rd16(REG_CMD_READ))
    ;
}

// recompute 'freespace'
void VCClass::getfree()
{
  uint16_t fullness = (wp - rd16(REG_CMD_READ)) & 4095;
  freespace = (4096 - 4) - fullness;
}

void VCClass::wpstart(size_t a)
{
  while (freespace < a)
    getfree();
  __wstart(RAM_CMD + wp);
}

void VCClass::wpbump(uint16_t a)
{
  wp = (wp + a) & 0xffc;
  wr16(REG_CMD_WRITE, wp);
  freespace -= (a & ~3);
}

void VCClass::copy(uint32_t addr, const uint8_t *src, size_t count)
{
  __wstart(addr);
  while (count--) {
    ft900_spim_transfer(*src++);
  }
  __end();
}

void VCClass::copycmds(const uint32_t *src, size_t count)
{
  while (count > 2048) {
    copycmds(src, 2048);
    src += (2048 / 4);
    count -= 2048;
  }
  while (freespace < count)
    getfree();
  copy(RAM_CMD + wp, (const uint8_t*)src, count);
  wpbump(count + 3);
}

extern "C" void uart_raw_tx(uint8_t); // This is the 'send byte' function

void VCClass::dumpscreen(bool wait)
{
  {
    uint32_t BIST_SCR = 0x1c2000;

    if (wait) {
      cmd_regread(0, 0);
      waitidle();
    }
    usleep(800 * 1000);

    wr(REG_RENDERMODE, 1);
    uart_raw_tx(0xa5);
    for (int ly = 0; ly < 272; ly++) {
      wr16(REG_SNAPY, ly);
      wr(REG_SNAPSHOT, 1);
      usleep(2 * 1000);
      while (rd32(REG_BUSYBITS) | rd32(REG_BUSYBITS + 4))
        ;
      wr(REG_BIST_EN, 1);
      __start(BIST_SCR);
      ft900_spim_transfer(0xff);
      for (int x = 0; x < 480; x++) {
        uart_raw_tx(ft900_spim_transfer(0xff));
        uart_raw_tx(ft900_spim_transfer(0xff));
        uart_raw_tx(ft900_spim_transfer(0xff));
        ft900_spim_transfer(0xff);
      }
      __end();
      wr(REG_BIST_EN, 0);
    }
    wr16(REG_RENDERMODE, 0);
  }
}
