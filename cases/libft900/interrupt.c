#include "libft900.h"
#include "libft900internal.h"

int ft900_enable_interrupts(void)
{
  return __sync_lock_test_and_set(ft900_intr_ctrl_3, 0);
}

int ft900_disable_interrupts(void)
{
  return __sync_lock_test_and_set(ft900_intr_ctrl_3, 0x80);
}

void ft900_restore_interrupts(int s)
{
  *ft900_intr_ctrl_3 = s;
}

