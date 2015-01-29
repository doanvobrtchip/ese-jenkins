#ifndef LIBFT900INTERNAL_H
#define LIBFT900INTERNAL_H

#include "ft900.h"
#define DEBUG ((volatile uint32_t*)0x1fff8);
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define MEMBAR() asm volatile ("" : : : "memory")

#define SPR2  0x20
#define SPR1  0x02
#define SPR0  0x01

#define DIV_4     (                 0)
#define DIV_8     (              SPR0)
#define DIV_16    (       SPR1       )
#define DIV_32    (       SPR1 | SPR0)
#define DIV_64    (SPR2              )
#define DIV_128   (SPR2        | SPR0)
#define DIV_256   (SPR2 | SPR1       )
#define DIV_512   (SPR2 | SPR1 | SPR0)

typedef struct {
  ft900_workbuf_t *w;
  size_t p;
} workbufptr_t;
#define BUMP(ptr) ((ptr).p = 0, (ptr).w = (ptr).w->__next)

void pad_setter(const __flash__ uint8_t *pads, size_t size, uint32_t f);
void streamout_pm_b(volatile uint8_t *dst, workbufptr_t *src, size_t n);

#endif
