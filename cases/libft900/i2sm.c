#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #define NDEBUG 1
#include <assert.h>

#include "libft900.h"
#include "libft900internal.h"

typedef struct {
  uint16_t config0;   // config register      
  uint16_t config2;   // clock setting register         
  uint16_t irqen;     // interrupt enable register          
  uint16_t irqpend;   // interrupt pending register (= FIFO status register)          
  uint16_t rwdata;    // transmit/receive data register 
  uint16_t __pad;     // 
  uint16_t rbyte;     // 
  uint16_t tbyte;     // 
} i2s_hw_t;

#define I2S ((volatile i2s_hw_t *)0x10350)

static volatile int isidle;

static struct {
  workbufptr_t tx;
  ft900_workbuf_t **append;
  int pending;
} i2sm;

// I2S FIFO status/interrupt bit masks

#define FIFOTUNDER 0x0001
#define FIFOTEMPTY 0x0002
#define FIFOTHALF  0x0004
#define FIFOTFULL  0x0008
#define FIFOTOVER  0x0010

#define FIFORUNDER 0x0100
#define FIFOREMPTY 0x0200
#define FIFORHALF  0x0400
#define FIFORFULL  0x0800
#define FIFOROVER  0x1000		

static void status()
{
  MEMBAR();
  iprintf("config0 %04x  config2 %04x  irqpend=%04x rbyte %d tbyte %d\n",
    I2S->config0, I2S->config2,
    I2S->irqpend,
    I2S->rbyte, I2S->tbyte);
  MEMBAR();
}

static void feed1()
{
  ft900_workbuf_t *wb = i2sm.tx.w;
  uint16_t *pd = wb->data + i2sm.tx.p;
  I2S->rwdata = *pd;
  i2sm.tx.p += 2;
  if (i2sm.tx.p == i2sm.tx.w->size) {
    BUMP(i2sm.tx);
    if (wb->completion)
      wb->completion(wb);
    if (!i2sm.tx.w)
      i2sm.append = &i2sm.tx.w;
  }
}

#define INTERRUPTS  (FIFOTHALF | FIFOTEMPTY)

static void i2s_isr()
{
  if (I2S->irqpend & INTERRUPTS) {
    // iprintf("ISR %04x %p\n", I2S->irqpend, i2sm.tx.w);
    // iprintf("%d,", I2S->tbyte);
    size_t fullness = I2S->tbyte;
    size_t empty = 1024 - I2S->tbyte;
    size_t tofill = (empty < 1008) ? empty : 1008;
    for (int i = 0; (i < tofill) && (i2sm.tx.w != NULL); i++)
      feed1();
    if (I2S->tbyte < 512)
      iprintf("%d\n", I2S->tbyte);

    I2S->irqpend = INTERRUPTS;
  }
}

void ft900_i2sm_activate()
{
  *sys_regclkcfg = *sys_regclkcfg | (1 << 1);
  static const __flash__ uint8_t pads[] = {
    60, 61, 62, 63, 64, 65, 66
  };
  pad_setter(pads, sizeof(pads), FT900_PAD_FUNC_1);

  I2S->config0 = 
    (1 << 5);
  I2S->config2 =
    7;
  I2S->irqpend = INTERRUPTS;
  attach_interrupt(15, i2s_isr);

  isidle = 1;
  i2sm.pending = 0;
  i2sm.append = &i2sm.tx.w;

  if (0) {
    for (int i = 0; i < 512; i++)
      I2S->rwdata = signal();
    I2S->irqpend = 0xffff;
    I2S->config0 |= 
      (1 << 6) |
      (1 << 0);
    // iprintf("RUN\n");
    // status();
    for (;;) {
      while ((I2S->irqpend & FIFOTHALF) == 0)
        ;
      for (int i = 0; i < 77; i++)
        I2S->rwdata = signal();
      I2S->irqpend = FIFOTHALF;
      // status();
      iprintf("%02x\n", I2S->irqpend & 0xff);
    }
  }
}

void ft900_i2sm_add(ft900_workbuf_t *wb)
{
  wb->__next = NULL;

  if (isidle) {
    i2sm.tx.w = wb;
    while (i2sm.tx.w && (i2sm.pending < 2048)) {
      feed1();
      i2sm.pending += 2;
    }
    if (i2sm.pending == 2048) {
      isidle = 0;
      I2S->irqpend = 0xffff;
      I2S->config0 |= (1 << 6) | (1 << 0);
      I2S->irqen = INTERRUPTS;
    }
  } else {
    int intstate = ft900_disable_interrupts();
    *i2sm.append = wb;
    if (i2sm.tx.w == NULL)
      i2sm.tx.w = wb;
    i2sm.append = &wb->__next;
    ft900_restore_interrupts(intstate);
  }
}
