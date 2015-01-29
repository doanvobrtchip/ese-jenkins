#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NDEBUG 1
#include <assert.h>

#include "libft900.h"
#include "libft900internal.h"

#include "ft900_test_notify.h"

/************************************************************************/

static volatile int isidle;

// Only accessed from inside the ISR

static struct {
  workbufptr_t rd, wr;
  ft900_workbuf_t **append;
  int8_t inflight;
} spim;

int ft900_spim_get_speed(void)
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

// wait until unit is idle - that is, no outstanding asynchronous
// transfers.

static void idle(void)
{
  while (!isidle)
    ;
}

void ft900_spim_set_speed(int sp)
{
  assert((2 <= sp) && (sp <= 512));
  assert(sp == (sp & -sp));   // only 1 bit can be set
  idle();
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

void ft900_spim_sel(uint8_t s)
{
  *spim_sscr = s;
}

void ft900_spim_unsel(void)
{
  *spim_sscr = 0xff;
}

#define WAIT_IDLE() \
  do { while ((*spim_spsr & 0x0c) != 0x0c) ; } while (0)

uint8_t ft900_spim_transfer(uint8_t v)
{
  idle();
  *spim_spdr = v;
  usleep(0);
  WAIT_IDLE();
  return *spim_spdr;
}

#if 0  // {
static void reportptr(volatile const workbufptr_t *p)
{
  iprintf("%p.%d ",
    p->w, p->p);
  if (p->w) {
    iprintf("data %p sz %d comp %p options %lx ",
      p->w->data, p->w->size, p->w->completion, p->w->options);
    // for (int i = 0; i < 4; i++) iprintf("%08x ", ((uint32_t*)(p->w))[i]);
  }
}

static void report(int line)
{
  iprintf("%3d: inflight=%d RD ", line, spim.inflight);
  reportptr(&spim.rd);
  iprintf(" WR ");
  reportptr(&spim.wr);
  iprintf("\n");
}

#endif  // }

static void slow_recv(size_t n)
{
  assert(spim.wr.w != NULL);
  spim.inflight -= n;
  while (n--) {
    uint8_t v = *spim_spdr;
    uint8_t *dst = (uint8_t*)spim.wr.w->data + spim.wr.p;
    if (spim.wr.w->options & FT900_WB_READ)
      *dst = v;
    spim.wr.p++;
    assert(spim.wr.p <= spim.wr.w->size);
    if (spim.wr.p == spim.wr.w->size) {
      ft900_workbuf_t *w = spim.wr.w;
      BUMP(spim.wr);
      if (w->completion)
        w->completion(w);
    }
  }
}

static void recv(size_t n)
{
  // Special case when not near the end of the buffer
  int reading = (spim.wr.w->options & FT900_WB_READ);
  if ((spim.wr.p + n) < spim.wr.w->size) {
    if (reading) {
      uint8_t *dst = (uint8_t*)spim.wr.w->data + spim.wr.p;
#if 0
      for (size_t i = 0; i < n; i++)
        *dst++ = *spim_spdr;
#else
      asm("streamin.b %0,%1,%2" \
          : \
          :"r"(dst), "r"(spim_spdr), "r"(n));
      dst += n;
#endif
    } else {
      for (size_t i = 0; i < n; i++)
        *spim_spdr;
    }
    spim.wr.p += n;
    spim.inflight -= n;
  } else {
    slow_recv(n);
  }
}


#define ASSERT_SPIM_VALID() do { \
  /* report(__LINE__); */ \
  if (spim.wr.w) \
    assert(spim.wr.w->size); \
  if (spim.rd.w) \
    assert(spim.rd.w->size); \
  assert((0 <= spim.inflight) && (spim.inflight <= 64)); \
  } while (0)


static void ticker(void)
{
  ASSERT_SPIM_VALID();
  if (*spim_spsr & 0x80) {
#if 0 // This removes more data, but is slow
    while (*spim_spsr & 0x80)
      recv(1);
    recv(31);
#else
    recv(32);
#endif
  }
  ASSERT_SPIM_VALID();

  if ((spim.inflight < 64) && (spim.rd.w != NULL)) {
    size_t n0 = 64 - spim.inflight;
    size_t n1 = spim.rd.w->size - spim.rd.p;
    size_t n = MIN(n0, n1);

    spim.inflight += n;
    if (spim.rd.w->options & FT900_WB_FLASH) {
#if 1
      streamout_pm_b(spim_spdr, &spim.rd, n);
#else
      const __flash__ uint8_t *src = spim.rd.w->flash_data;
      for (size_t i = 0; i < n; i++)
        *spim_spdr = src[spim.rd.p + i];
#endif
    } else if (spim.rd.w->options & FT900_WB_WRITE) {
      const uint8_t *src = spim.rd.w->data;
      // for (size_t i = 0; i < n; i++)
      //   *spim_spdr = src[spim.rd.p + i];
      asm("streamout.b %0,%1,%2" \
          : \
          :"r"((volatile uint8_t *)spim_spdr), "r"(src + spim.rd.p), "r"(n));
    } else {
      for (size_t i = 0; i < n; i++)
        *spim_spdr = 0xff;
    }
    spim.rd.p += n;
    if (spim.rd.p == spim.rd.w->size)
      BUMP(spim.rd);
  }
  ASSERT_SPIM_VALID();

  // SPSR bits 2 and 3 high means that transfer is finished.
  // Can now finish the transaction by draining the read FIFO

  if (((*spim_spsr & 0x0c) == 0x0c) && (spim.rd.w == NULL)) {
    if (spim.inflight > 0)
      recv(spim.inflight);

    assert(spim.wr.w == NULL);
    assert(spim.rd.w == NULL);

    spim.append = &spim.rd.w;

    isidle = 1;
    *spim_stfcr &= ~0x08;  // clear TXIEN
  }
  ASSERT_SPIM_VALID();
}

void ft900_spim_add(ft900_workbuf_t *wb)
{
  assert(wb->size != 0);
  wb->__next = NULL;

  assert(wb->size != 0);

  int intstate = ft900_disable_interrupts();
  *spim.append = wb;
  if (spim.rd.w == NULL)
    spim.rd.w = wb;
  if (spim.wr.w == NULL)
    spim.wr.w = wb;
  spim.append = &wb->__next;
  isidle = 0;
  ft900_restore_interrupts(intstate);

  *spim_stfcr |= 0x08;  // TXIEN
}

#if 0
void ft900_spim_add1(ft900_workbuf_t *wb)
{
  wb->__next = NULL;
  *spim.append = wb;
  spim.append = &wb->__next;
}

void ft900_spim_add2(ft900_workbuf_t *wb0, ft900_workbuf_t *wb1)
{
  assert(ft900_spim_is_idle());
  ft900_spim_add1(wb0);
  ft900_spim_add1(wb1);

  if (spim.rd.w == NULL)
    spim.rd.w = wb0;
  if (spim.wr.w == NULL)
    spim.wr.w = wb0;

  isidle = 0;
  *spim_stfcr |= 0x08;  // TXIEN
}
#endif

int ft900_spim_is_idle()
{
  return isidle;
}

void ft900_spim_activate()
{
  // Enable SPIM function (bit 7)
  *sys_regclkcfg = *sys_regclkcfg | (1 << 7);
  spim.append = &spim.rd.w;

  *spim_spcr =
               (1 << 7)   // SPIE
             | (1 << 6)   // SPE (SPI system enable)
             | (1 << 4)   // MSTR (Master mode select)
             | DIV_8;   
  *spim_sfcr |= (2 << 6) | 0x27;         // EN (FIFO enable), 64 byte, and clear fifos

  // Enable pads 
  // 27 SPIM SCK
  // 28 SPIM SS0
  // 29 SPIM MOSI
  // 30 SPIN MISO
  // 33 SPIM SS1
  // 35 SPIM SS3

  static const __flash__ uint8_t pads[] = {
    27, 28, 29, 30,
    33, 34 };
  pad_setter(pads, sizeof(pads), FT900_PAD_FUNC_1);

  isidle = 1;
  attach_interrupt(SPIM_IRQ, ticker);
}
