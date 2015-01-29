#include <stdint.h>
#include "ft900.h"
#include "libft900.h"

void ft900_init()
{
  // Peripheral reset
  if (1) {
    uint8_t b = *sys_regmsc0cfg_b3;
    *sys_regmsc0cfg_b3 = b | 0x80;
    *sys_regmsc0cfg_b3 = b;
  }

  ft900_uart_activate(0);
  // ft900_dmac_init();
  // *sys_regclkcfg = *sys_regclkcfg & ~0x0010;
  // *(uint8_t*)0x100e3 = 0x80;  // interrupts off
}

extern void _exithook(void);

void ft900_exit()
{
  static const __flash__ char eot[4] = "%H%\n";
  extern void uart_tx(char);
  extern int uart_rx();

  _exithook();
  for (int i = 0; i < sizeof(eot); i++)
    uart_tx(eot[i]);
  uart_rx();
  asm("jmp 0");
}

void ft900_watchdog()
{
  static const __flash__ char eot[4] = "%W%\n";
  extern void uart_tx(char);
  extern int uart_rx();

  for (int i = 0; i < sizeof(eot); i++)
    uart_tx(eot[i]);
  uart_rx();
  asm("jmp 0");
}
