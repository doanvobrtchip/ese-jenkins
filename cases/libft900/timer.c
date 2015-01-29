#include <stdint.h>
#include "ft900.h"

#include "libft900.h"

typedef void(*fptr)(void);
static fptr handlers[4] = { NULL, NULL, NULL, NULL };

static void timer_handler()
{
  uint8_t fired = *timer_int & 0x55;
  for (int i = 0; i < 4; i++) {
    if (fired & (1 << (2 * i))) {
      fptr b = handlers[i];
      if (b != NULL)
        b();
    }
  }
  *timer_int |= fired;  // clear the interrupts
}


void ft900_timer_activate()
{
  attach_interrupt(17, timer_handler);
  
  //Enable timer bit 1 block-en, bit 0 soft-reset
  *timer_control_0 = 0x03;
  *timer_control_0 = 0x02;
}

uint32_t ft900_timer_get_prescaler()
{
  return (*timer_presc_ls | (*timer_presc_ms << 8)) + 1;
}

void ft900_timer_set_prescaler(uint32_t d)
{
  d -= 1;
  *timer_presc_ls = d & 0xff;
  *timer_presc_ms = (d >> 8) & 0xff;
  //clear the prescaler
  *timer_control_4 = (1 << 4);
}

uint32_t ft900_timer_get_value(uint8_t ti)
{
  *timer_select = ti << 2;
  uint32_t r = *timer_read_ls;
  r |= (*timer_read_ms << 8);
  return r;
}

void ft900_timer_set_value(uint8_t ti, uint32_t d)
{
  *timer_select = ti;
  d -= 1;
  *timer_write_ls = d & 0xff;
  *timer_write_ms = (d >> 8) & 0xff;
}

void ft900_timer_set_prescaling(uint8_t ti, int ps)
{
  uint8_t mask = 0x10 << ti;
  if (ps)
    *timer_control_2 |= mask;
  else
    *timer_control_2 &= ~mask;
}

int ft900_timer_get_prescaling(uint8_t ti)
{
  uint8_t mask = 0x10 << ti;
  return (*timer_control_2 & mask) != 0;
}

void ft900_timer_set_mode(uint8_t ti, uint8_t mode)
{
  uint8_t mask = 0x11 << ti;
  mode <<= ti;
  *timer_control_3 = (*timer_control_3 & mask) | mode;
}

uint8_t ft900_timer_get_mode(uint8_t ti)
{
  return (*timer_control_3 >> ti) & 0x11;
}

void ft900_timer_set_interrupt(uint8_t ti, void (*func)())
{
  uint8_t mask = 0x02 << (2 * ti);
  if (func != NULL)
    *timer_int |= mask;
  else
    *timer_int &= ~mask;
  handlers[ti] = func;
}

void ft900_timer_start(uint8_t ti)
{
  uint8_t mask = 0x01 << ti;
  *timer_control_4 = mask;  // clear timer
  *timer_control_1 = mask;  // start timer
}

void ft900_timer_stop(uint8_t ti)
{
  uint8_t mask = 0x10 << ti;
  *timer_control_1 = mask;  // stop timer
}
