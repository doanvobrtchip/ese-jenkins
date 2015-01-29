#include "libft900.h"

struct pmcontrol
{
  uint32_t unlock;
  uint32_t address;
  uint32_t data;
};

#define PMCONTROL (*(volatile struct pmcontrol*)0x1fc80)

void ft900_pm_write(uint32_t dst, const void *src, size_t s)
{
  PMCONTROL.unlock = 0x1337f7d1;
  PMCONTROL.address = dst;
  asm("streamout.l %0,%1,%2" \
      : \
      :"r"(&PMCONTROL.data), "r"(src), "r"(s));
  PMCONTROL.unlock = 0x00000000;
}

void ft900_pm_read(void *dst, uint32_t src, size_t s)
{
  __flash__ uint32_t* ps = (__flash__ uint32_t*)src;
  uint32_t *pd = (uint32_t *)dst;
  uint32_t *pd1 = pd + (s >> 2);
  while (pd != pd1)
    *pd++ = *ps++;
}
