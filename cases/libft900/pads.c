#include "libft900.h"

static uint8_t *PADS = (uint8_t *)0x1001c;

uint8_t ft900_pad_get_function(uint8_t pad)
{
  return PADS[pad];
}

void ft900_pad_set_function(uint8_t pad, uint8_t func)
{
  PADS[pad] = func;
}
