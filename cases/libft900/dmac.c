#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ft900.h"
#include "libft900.h"

// #define DEBUG_SPEW

#define USE_STREAM  1
#define USE_THR     0

typedef struct {
  union {
    uint8_t isr;  // read-only
    uint8_t iack; // write-only
  };
  uint8_t imr;
  uint8_t rcr;
  uint8_t tcr;
  uint32_t dr;
  uint8_t iar[6];
  uint8_t thr;
  uint8_t mcr;
  uint8_t mdvr;
  uint8_t mar;
  uint16_t mdtx;
  uint16_t mdrx;
  uint8_t npr;
  uint8_t trr;
} DMAC_t;

#define dmac ((DMAC_t volatile *)0x10220)

#define MCR_START   0x01
#define MCR_WRITE   0x02

#define ISR_MDINT   0x20

uint16_t ft900_dmac_mii_read(uint8_t reg)
{
  dmac->mcr = (reg << 3) | MCR_START;
  while ((dmac->isr & ISR_MDINT) == 0)
    ;
  dmac->iack = ISR_MDINT;
  return dmac->mdrx;
}

void ft900_dmac_mii_write(uint8_t reg, uint16_t v)
{
  // iprintf("Write %04x to %02x\n", v, reg);
  if ((dmac->isr & ISR_MDINT) != 0) {
    iprintf("MDINT not clear\n");
    exit(0);
  }
  dmac->mdtx = v;
  dmac->mcr = (reg << 3) | MCR_START | MCR_WRITE;
  while ((dmac->isr & ISR_MDINT) == 0)
    ;
  dmac->iack = ISR_MDINT;
}

#ifdef DEBUG_SPEW
static void dump_regs(void)
{
  static char* names[] = {
    "ISR",
    "IMR",
    "RCR",
    "TCR",
    "DR0",
    "DR1",
    "DR2",
    "DR3",
    "IAR0",
    "IAR1",
    "IAR2",
    "IAR3",
    "IAR4",
    "IAR5",
    "THR",
    "MCR",
    "MDVR",
    "MAR",
    "MDTX0",
    "MDTX1",
    "MDRX0",
    "MDRX1",
    "NPR",
    "TRR"
  };

  for (int i = 0; i < 0x18; i++)
    iprintf("%02x: %5s %02x\n", i, names[i], ((uint8_t*)dmac)[i]);
}

static void dump_phy_regs(void)
{
  iprintf("PHY:\n");
  for (int i = 0; i < 32; i++) {
    iprintf("%2d: %04x\n", i, ft900_dmac_mii_read(i));
  }
}
#endif

void ft900_dmac_init(void)
{
  *sys_regphymsc_b0 = *sys_regphymsc_b0 | 0x8;
  // Set the Ethernet PHY Address as a global variable
  // *dmac_phy_address = 0x8;
  // Keep holding the PHY reset
  *sys_regmsc0cfg_b1 = *sys_regmsc0cfg_b1 | 0x01;
  // Switch on DMAC PHY clock (bit 13)
  *sys_regclkcfg = *sys_regclkcfg | 0x2000;

  // Set delay to ensure clocks (asicpin and mdc) are provided to the PHY
  usleep(200000);

  // Switch off DMAC PHY pwrsv and pwrdn
  *sys_regphymsc_b1 = *sys_regphymsc_b1 & 0xfc;
  usleep(200000); // added delay, just in case
  // Release the PHY reset
  *sys_regmsc0cfg_b1 = *sys_regmsc0cfg_b1 & 0xfe;

  dmac->mar = 23;
  dmac->mdvr = 255;

#ifdef DEBUG_SPEW
    for (int i = 0; i < 32; i++) {
      dmac->mar = i;
      iprintf("%2d: %04x\n", i, ft900_dmac_mii_read(0));
    }
    iprintf("\n");
#endif

  static __flash__ uint8_t mac[] = {0x00, 0x12, 0x34, 0x56, 0x78, 0x9a};

  uint16_t status;
  do {
    status = ft900_dmac_mii_read(1);
  } while ((status & 4) == 0);
#ifdef DEBUG_SPEW
  iprintf("Link up: %04x\n", status);
#endif

  dmac->rcr = 0xf;
  dmac->tcr = 0x17;
#if USE_THR
  dmac->thr = 0x2;    // Start early TX after 136 bytes
#endif

#ifdef DEBUG_SPEW
  dump_regs();
  dump_phy_regs();
#endif

  for (int i = 0; i < 6; i++)
    dmac->iar[i] = mac[i];
#ifdef DEBUG_SPEW
  iprintf("MAC is %02x:%02x:%02x:%02x:%02x:%02x\n",
    dmac->iar[0],
    dmac->iar[1],
    dmac->iar[2],
    dmac->iar[3],
    dmac->iar[4],
    dmac->iar[5]
    );
#endif
}

int ft900_dmac_input(size_t *blen, uint8_t *buf)
{
  uint8_t *pb;

  if (dmac->npr == 0)
    return 0;

  uint32_t w0 = dmac->dr;
  int16_t len = (w0 & 0xffff);
  *blen = len;
  len -= 4;
  pb = buf + 2;
  *pb++ = w0 >> 16;
  *pb++ = w0 >> 24;
#if 0 || !USE_STREAM
  while (len > 0) {
    uint32_t d = dmac->dr;
    *pb++ = d >> 0;
    *pb++ = d >> 8;
    *pb++ = d >> 16;
    *pb++ = d >> 24;
    len -= 4;
  }
#else
  uint32_t *dst = (uint32_t*)pb;
  asm("streamin.l %0,%1,%2" \
      : \
      :"r"(dst), "r"(&(dmac->dr)), "r"(len));
#endif
  // iprintf("recv pkt %d\n", *blen);
  return 1;
}

int ft900_dmac_output(size_t blen, uint8_t *buf)
{
  // iprintf("send pkt %d\n", blen);
  // wait for any previous transmit to complete
  while (dmac->trr)
    ;

  // clear write data pointer
  dmac->iack = 0x02;

  // why is this needed?
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");
  asm(" move.l $r0,$r0");

#if 0
  // write packet
  for (int i = 0; i < blen; i += 4) {
    uint32_t d = *(uint32_t*)(buf + i);
    // iprintf("OUT %3d: %08x\n", i, d); millisleep(1);
    dmac->dr = d;
  }
#else

#if !USE_STREAM
  uint32_t *src = (uint32_t*)buf;
  uint32_t *end = src + ((blen + 3) / 4);
  while (src < end)
    dmac->dr = *src++;
#else
  uint32_t *src = (uint32_t*)buf;
  asm("streamout.l %0,%1,%2" \
      : \
      :"r"(&(dmac->dr)), "r"(src), "r"(blen + 3));
#endif

#endif

#if USE_THR
  if (dmac->trr == 0)
    dmac->trr = 1;
#else
  dmac->trr = 1;
#endif

  return 1;
}
