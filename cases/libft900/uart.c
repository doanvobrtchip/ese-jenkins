#include <stdio.h>
#include <stdint.h>
#include "libft900.h"
#include "libft900internal.h"

#define DEBUGGING 0
#if DEBUGGING
int trace[100], traceidx;
#endif

#define ACR 0x00 // Additional Control Register
#define CPR 0x01 // Clock Prescaler Register
#define TCR 0x02 // Time Clock Register
#define CKS 0x03 // Clock Select Register
#define TTL 0x04 // Transmitter Trigger Level
#define RTL 0x05 // Receiver Trigger Level
#define FCL 0x06 // Flow Control Level (low)
#define FCH 0x07 // Flow Control Level (high)
#define ID1 0x08 // Identification Register 1
#define ID2 0x09 // Identification Register 2
#define ID3 0x0A // Identification Register 3
#define REV 0x0B // Revision Register
#define CSR 0x0C // Channel Software Register
#define NMR 0x0D // Nine-bit mode Register
#define MDM 0x0E // Modem Disable Mask register
#define RFC 0x0F // FCR register read
#define GDS 0x10 // Good Data Status register
#define DMS 0x11 // DMA register
#define CKA 0x13 // Clock Alteration register

typedef struct {
  //                       R    W   650  950  lcr7
  /* 0 */ union { uint8_t rhr, thr,           dll    ; };
  /* 1 */ union { uint8_t ier,           asr, dlm    ; };
  /* 2 */ union { uint8_t isr, fcr, efr              ; };
  /* 3 */ union { uint8_t lcr,           rfl         ; };
  /* 4 */ union { uint8_t mcr,           tfl         ; };
  /* 5 */ union { uint8_t lsr,           icr         ; };
  /* 6 */ union { uint8_t msr                        ; };
  /* 7 */ union { uint8_t spr                        ; };
  uint8_t __pad[8];
} uart_hw_t;

#define UARTS ((volatile uart_hw_t *)0x10320)

#define FIFOSIZE    128   // Always running in 128 byte FIFO mode
#define TX_THRESH   32    // Interrupt when TX FIFO reaches this

typedef struct {
  workbufptr_t tx, rx;
  ft900_workbuf_t **append_tx;
  int8_t inflight;
} uart_t;
static uart_t uarts[2];

static void write_indexed(volatile uart_hw_t *puh, uint8_t reg, uint8_t val)
{
  puh->spr = reg;
  puh->icr = val;
  MEMBAR();
}

#define ACR_VAL    (1 << 5)

#if DEBUGGING
static uint8_t read_indexed(volatile uart_hw_t *puh, uint8_t reg)
{
  write_indexed(puh, ACR, (1 << 6) | ACR_VAL);
  puh->spr = reg;
  uint8_t r = puh->icr;
  write_indexed(puh, ACR, (0 << 6) | ACR_VAL);
  return r;
}
#endif

#define SWAPLCR(puh, n) __sync_lock_test_and_set((puh)->lcr, n)

static void handler(volatile uart_hw_t *puh, uart_t *pu)
{
  uint8_t isr = puh->isr;
  switch (isr & 0x3f) {
    case 0x02:  // Transmitter empty
      {
        size_t n0 = FIFOSIZE - TX_THRESH;
        size_t n1 = pu->tx.w->size - pu->tx.p;
        size_t n = MIN(n0, n1);

        if (pu->tx.w->options & FT900_WB_FLASH) {
          streamout_pm_b(&puh->thr, &pu->tx, n);
        } else if (pu->tx.w->options & FT900_WB_WRITE) {
          asm("streamout.b %0,%1,%2" \
              : \
              :"r"(&puh->thr), "r"(pu->tx.w->data + pu->tx.p), "r"(n));
        }
        // write_indexed(puh, ACR, (1 << 7) | ACR_VAL);
        // trace[traceidx++] = puh->tfl;
        // write_indexed(puh, ACR, (0 << 7) | ACR_VAL);

        pu->tx.p += n;
        if (pu->tx.p == pu->tx.w->size)
          BUMP(pu->tx);
        if (pu->tx.w == NULL) {
          puh->ier &= ~2;
          pu->append_tx = &pu->tx.w;
        }
      }
      break;

    default:
      break;
  }
}

static void handler0(void)
{
  handler(&UARTS[0], &uarts[0]);
}

static void handler1(void)
{
  handler(&UARTS[1], &uarts[1]);
}

void ft900_uart_activate(size_t uart)
{
  // Enable UART0 function (bit 4)
  // Enable UART1 function (bit 3)
  *sys_regclkcfg |= (1 << (uart ? 3 : 4));

  static const __flash__ uint8_t pads[] = {
    48, 49, 50, 51, 52, 53, 54, 55
    };
  pad_setter(pads, sizeof(pads), FT900_PAD_FUNC_3);

  // Interrupt select and clock select switch on
  *sys_regmsc0cfg_b2 = 0x0;
  *sys_regmsc0cfg_b2 = *sys_regmsc0cfg_b2 | 0x28;

  ft900_uart_set_line(uart, FT900_UART_8BIT);
  ft900_uart_set_baud(uart, 115200);
  volatile uart_hw_t *puh = &UARTS[uart];
  uart_t *pu = &uarts[uart];

  uint8_t saved_lcr = puh->lcr;
  puh->lcr = 0xbf;
  MEMBAR();
  puh->efr |= (1 << 4);
  puh->lcr = saved_lcr;
  MEMBAR();

  puh->fcr = 0x07;
  puh->mcr = 2;

  write_indexed(puh, ACR, ACR_VAL);  // interrupt on levels TTL, RTL
  write_indexed(puh, TTL, TX_THRESH);
  write_indexed(puh, RTL, TX_THRESH);

  pu->append_tx = &pu->tx.w;
  pu->tx.w = NULL;
  pu->tx.p = 0;


  typedef void (*funcptr_t)(void);
  static const __flash__ funcptr_t handlers[2] = {handler0, handler1};
  attach_interrupt(13 + uart, handlers[uart]);
}

// See p.30 of D16950 spec. The algorithm for computing
// the best approximation to the baud rate is as follows:
//
// Set prescaler (CPR) to 1, so that MCR[7] is don't-care.
// Set TCR to 4, and compute 16-bit divisor.
// If divisor overflows, set TCR to 16 and adjust divisor.
// Load divisor into (DLM, DLL)

void ft900_uart_set_baud(size_t uart, uint32_t baud)
{
  volatile uart_hw_t *puh = &UARTS[uart];

  write_indexed(puh, CPR, 1 << 3);

  uint32_t divisor = FT900_SYSTEM_CLOCK / (4 * baud);
  if (divisor < 65536) {
    write_indexed(puh, TCR, 4);
  } else {
    write_indexed(puh, TCR, 0);  // means SC divides by 16
    divisor >>= 2;
  }

  uint8_t saved_lcr = puh->lcr;
  puh->lcr = 0x80;
  MEMBAR();
  puh->dll = divisor;
  puh->dlm = divisor >> 8;
  puh->lcr = saved_lcr;
}

void ft900_uart_set_line(size_t uart, uint8_t lc)
{
  volatile uart_hw_t *puh = &UARTS[uart];
  puh->lcr = lc;
}

void ft900_uart_add(size_t uart, ft900_workbuf_t *wb)
{
  uart_t *pu = &uarts[uart];
  volatile uart_hw_t *puh = &UARTS[uart];

  wb->__next = NULL;

  int intstate = ft900_disable_interrupts();
  *pu->append_tx = wb;
  pu->append_tx = &wb->__next;
  ft900_restore_interrupts(intstate);

  puh->ier = 0x02;
}

void ft900_uart_transmit(size_t uart, uint8_t b)
{
  volatile uart_hw_t *puh = &UARTS[uart];
  // Wait until LSR6 is high, indicating XMIT FIFO empty
  while ((puh->lsr & (1 << 6)) == 0)
    ;
  puh->thr = b;
  // write_indexed(puh, ACR, 0x80);
}

#if DEBUGGING
void ft900_spill()
{
  ft900_disable_interrupts();
  iprintf("\n\n--- TRACE ---\n");
  for (int i = 0; i < traceidx; i++)
    iprintf("%d: %08x\n", i, trace[i]);
}
#endif
