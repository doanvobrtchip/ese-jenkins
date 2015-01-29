#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NDEBUG 1
#include <assert.h>

#include "libft900.h"
#include "libft900internal.h"

typedef struct {
  uint32_t control;
  uint32_t fullness;
  uint32_t data;
  uint32_t options;
} DCAP_t;

#define OPTION_IE   0x1   // field in 'options' above

static volatile int isidle;
static struct {
  ft900_workbuf_t *wr;
  ft900_workbuf_t **append;
} dcap;

#define DCAP (*(DCAP_t volatile *)0x10360)

void new_head()
{
  if (dcap.wr) {
    uint32_t thresh = dcap.wr->size;
    DCAP.control = (DCAP.control & 0xfffff000) | thresh;
    DCAP.options |= OPTION_IE;
    isidle = 0;
  } else {
    DCAP.options &= ~OPTION_IE;
    isidle = 1;
    dcap.append = &dcap.wr;
  }
}

static void handler(void)
{
  if (DCAP.options & 4) {
    asm("streamin.l %0,%1,%2" \
        : \
        :"r"(dcap.wr->data), "r"(&(DCAP.data)), "r"(dcap.wr->size));
    ft900_workbuf_t *w = dcap.wr;
    dcap.wr = w->__next;
    if (w->completion != NULL)
      w->completion(w);
    new_head();
  }
}

void ft900_dcap_activate(void)
{
  // Enable camera module (DCAP), bit 0
  *sys_regclkcfg = *sys_regclkcfg | (1 << 0);

  static const __flash__ uint8_t pads[] = {
   6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    };
  pad_setter(pads, sizeof(pads), FT900_PAD_FUNC_1);

  dcap.wr = NULL;
  attach_interrupt(7, handler);
  new_head();
}

void ft900_dcap_set_count(uint32_t v)
{
  DCAP.control = (DCAP.control & 0x0000ffff) | (v << 16);
}

void ft900_dcap_set_trigpat(uint32_t v)
{
  DCAP.control = (DCAP.control & 0xffff0fff) | (v << 12);
}

void ft900_dcap_set_clock_polarity(uint32_t v)
{
  DCAP.options = (DCAP.options & ~2) | (v << 1);
}

void ft900_dcap_add(ft900_workbuf_t *wb)
{
  assert(wb->size != 0);
  wb->__next = NULL;

  int intstate = ft900_disable_interrupts();
  *dcap.append = wb;
  dcap.append = &wb->__next;
  if (isidle)
    new_head();
  ft900_restore_interrupts(intstate);
}

int ft900_dcap_is_idle(void)
{
  return isidle;
}

void ft900_dcap_drain()
{
  uint32_t __attribute__((unused)) dummy;
  while (DCAP.fullness) 
    dummy = DCAP.data;
}
