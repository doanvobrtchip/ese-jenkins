#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include "libft900.h"
#include "libft900internal.h"

__attribute__ ((unused)) static void ticker()
{
  *spim_sscr = 0xff;
  __attribute__ ((unused)) uint8_t spsr = *spim_spsr;
  __attribute__ ((unused)) uint8_t rx = *spim_spdr;
  // iprintf("spsr=%02x spdr=%02x .. %02lx \n", spsr, rx, *spim_spsr);
}

void spi_begin()
{
  // Enable SPIM function (bit 7)
  *sys_regclkcfg = *sys_regclkcfg | (1 << 7);
  usleep(1000);

  // iprintf("spim_spcr= %p %02lx\n", spim_spcr, *spim_spcr);
  // iprintf("spim_spsr= %p %02lx\n", spim_spsr, *spim_spsr);
  // iprintf("\n");


  *spim_spcr =
               (0 << 7)   // SPIE
             | (1 << 6)   // SPE (SPI system enable)
             | (1 << 4)   // MSTR (Master mode select)
             | DIV_8;   

#if 0
  *spim_sfcr = 1;         // EN (FIFO enable)
#endif

#if 0
  *spim_stfcr = 0x80;     // FIFO EXT (Enable wide FIFO writes)
#endif

  // iprintf("spim_spcr= %p %02lx\n", spim_spcr, *spim_spcr);
  // iprintf("spim_spsr= %p %02lx\n", spim_spsr, *spim_spsr);
  // iprintf("spim_sfcr= %p %02lx\n", spim_sfcr, *spim_sfcr);

  // Enable pads 
  // 27 SPIM SCK
  // 29 SPIM MOSI
  // 30 SPIN MISO
  // 33 SPIM SS1
  // 35 SPIM SS3

  *sys_regpad27 = *sys_regpad27 | 0x40; 
  *sys_regpad29 = *sys_regpad29 | 0x40; 
  *sys_regpad30 = *sys_regpad30 | 0x40; 
  *sys_regpad33 = *sys_regpad33 | 0x40; 
  // *sys_regpad35 = *sys_regpad35 | 0x40; 

  *sys_regpad27 = *sys_regpad27 | 0x40; 
  *sys_regpad28 = *sys_regpad28 | 0x40; 
  *sys_regpad29 = *sys_regpad29 | 0x40; 
  *sys_regpad30 = *sys_regpad30 | 0x40; 
  // *sys_regpad31 = *sys_regpad31 | 0x40; 
  // *sys_regpad32 = *sys_regpad32 | 0x40; 
  *sys_regpad33 = *sys_regpad33 | 0x40; 
  *sys_regpad34 = *sys_regpad34 | 0x40; 
  // *sys_regpad35 = *sys_regpad35 | 0x40; 

#if 0
  attach_interrupt(SPIM_IRQ, ticker);
  *ft900_intr_ctrl_3 = 0;
  printf("Int mask is %02x\n", *ft900_intr_ctrl_3);
#endif

  return;

#if 0
  for (;;) {
    *spim_sscr = 0xf7;
    *spim_spdr = 0x55;
    while ((*spim_spsr & 0x4) == 0)
      ;
    usleep(500);
    *spim_sscr = 0xff;
    iprintf("spim_spsr = %p %02lx\n", spim_spsr, *spim_spsr);
    iprintf("    %02x\n", *spim_spdr);
    iprintf("    spim_spsr= %p %02lx\n", spim_spsr, *spim_spsr);
  }
#endif
}

int spi_get_speed(void)
{
  if (*spim_stfcr & (1 << 4)) {
    return 2;
  } else {
    int r = 4;
    if (*spim_spcr & SPR0)
      r <<= 1;
    if (*spim_spcr & SPR1)
      r <<= 2;
    if (*spim_spcr & SPR2)
      r <<= 4;
    return r;
  }
}

void spi_set_speed(int sp)
{
  if (sp == 2) {
    *spim_stfcr |= (1 << 4);
  } else {
    *spim_stfcr &= ~(1 << 4);
    uint8_t c = *spim_spcr & ~(SPR0 | SPR1 | SPR2);

    if (sp & (              64 | 128 | 256 | 512))
      c |= SPR2;
    if (sp & (    16 | 32 |            256 | 512))
      c |= SPR1;
    if (sp & (8 |      32 |      128 |       512))
      c |= SPR0;

    *spim_spcr = c;
  }
}

void spi_ss(int s)
{
  *spim_sscr = s ? 0xff : 0xfe;
}

void spi_sel(uint8_t s)
{
  *spim_sscr = s;
}

void spi_unsel(void)
{
  *spim_sscr = 0xff;
}

#define WAIT_IDLE() \
  do { while ((*spim_spsr & 0x8c) != 0x8c) ; } while (0)

uint8_t spi_transfer(uint8_t v)
{
  *spim_spdr = v;
  WAIT_IDLE();
  return *spim_spdr;
}

void spi_bulk_write(const uint8_t *pd, size_t s)
{
  *spim_sfcr |= 7;         // EN (FIFO enable)

  const uint8_t *pe = pd + s;

  if (0) {
    while (pd < pe) {
      while ((*spim_spsr & 0x08) == 0)
        ;
      for (int i = 0; i < 16; i++)
        *spim_spdr = *pd++;
    }
  } else {
    const uint32_t *pd32 = (const uint32_t *)pd;
    const uint32_t *pe32 = (const uint32_t *)pe;

    *spim_stfcr |= 0x80;  // wide FIFO writes
    while (pd32 < pe32) {
      WAIT_IDLE();
#if 1
      for (int i = 0; i < 16; i++)
        *spim_spdr = pd32[i];
#else

      for (int i = 0; i < 16; i += 2)
      asm("streamout.l %0,%1,%2" \
          : \
          :"r"(spim_spdr), "r"(pd32 + i), "r"(8));
#endif
      pd32 += 16;
    }
    // Wait for SPSR.TEMT to go high, meaning FIFO empty
    while ((*spim_spsr & 0x4) == 0)
      ;
    *spim_stfcr &= ~0x80;
  }

  // Wait for SPSR.TEMT to go high, meaning FIFO empty
  while ((*spim_spsr & 0x4) == 0)
    ;

  *spim_sfcr &= ~1;         // FIFO disable
}
