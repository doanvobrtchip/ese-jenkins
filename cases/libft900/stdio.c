#include <stdio.h>
#include <stdint.h>
#include "ft900.h"

#define UART1_LSR_DR   (1 << 0)
#define UART1_LSR_THRE (1 << 5)

void uart_raw_tx(uint8_t b)
{
  asm volatile ("" : : : "memory");
  while ((*uart1_lsr & UART1_LSR_THRE) == 0)
    ;
  asm volatile ("" : : : "memory");
  *uart1_thr = b;
  asm volatile ("" : : : "memory");
}

void uart_tx(uint8_t b)
{
  if (b == '\n')
    uart_raw_tx('\r');
  uart_raw_tx(b);
}

uint8_t uart_rx()
{
  while ((*uart1_lsr & UART1_LSR_DR) == 0)
    ;
  return *uart1_rhr;
}

ssize_t _write(int fd, const void *buf, size_t count)
{
  for (size_t i = 0; i < count; i++)
    uart_tx(((const uint8_t*)buf)[i]);
  return count;
}

ssize_t _read(int fd, void *buf, size_t count)
{
  char *pb = (char*)buf;
  pb[0] = uart_rx();
  return 1;
}

