#include "libft900.h"

static uint8_t *GPIO_CONF = (uint8_t *)0x10060;

uint8_t ft900_gpio_get_direction(uint8_t pad)
{
  volatile uint8_t *iob = GPIO_CONF + pad / 2;
  if ((pad & 1) == 0)
    return 4 & *iob;
  else
    return 4 & (*iob >> 4);
}

void ft900_gpio_set_direction(uint8_t pad, uint8_t dir)
{
  volatile uint8_t *iob = GPIO_CONF + pad / 2;
  if ((pad & 1) == 0)
    *iob = (*iob & 0xf0) | (dir);
  else
    *iob = (*iob & 0x0f) | (dir << 4);
}

void ft900_gpio_write(uint8_t pad, uint8_t val)
{
  volatile uint8_t *valp = (uint8_t *)0x10084 + (pad >> 3);
  uint8_t bit = pad & 7;
  uint8_t mask = ~(1 << bit);
  *valp = ((*valp) & mask) | (val << bit);
}

uint8_t ft900_gpio_read(uint8_t pad)
{
  volatile uint8_t *valp = (uint8_t *)0x10084 + (pad >> 3);
  uint8_t bit = pad & 7;
  return (*valp >> bit) & 1;
}
