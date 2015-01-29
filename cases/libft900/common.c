#include "libft900.h"
#include "libft900internal.h"

void pad_setter(const __flash__ uint8_t *pads, size_t size, uint32_t f)
{
  for (size_t i = 0; i < size; i++)
    ft900_pad_set_function(pads[i], f);
}


void streamout_pm_b(volatile uint8_t *dst, workbufptr_t *src, size_t n)
{
  const __flash__ uint8_t *ps = src->w->flash_data + src->p;
  for (size_t i = 0; i < n; i++)
    *(dst) = *ps++;
}
