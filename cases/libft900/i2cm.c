#include "libft900.h"
#include "libft900internal.h"

typedef struct {
  //                       R    W   
  /* 0 */ union { uint8_t    sa     ; };
  /* 1 */ union { uint8_t  sr,  cr  ; };
  /* 2 */ union { uint8_t    buf    ; };
  /* 3 */ union { uint8_t    tp     ; };
  /* 4 */ union { uint8_t    bl     ; };
  /* 5 */ union { uint8_t    ie     ; };
  /* 6 */ union { uint8_t    pend   ; };
  /* 7 */ union { uint8_t    fifo   ; };
  /* 8 */ union { uint8_t    string ; };
} i2cm_hw_t;

#define I2CM ((volatile i2cm_hw_t *)0x10300)

static volatile int flag;
static uint8_t i2cm_address;
static size_t  i2cm_count;

static void i2c_handler()
{
  I2CM->pend = 0x40;
  flag = 1;
}

#define BUSY  1
#define ERROR 2

#ifdef VERBOSE
static void status()
{
  char *names[] = {"   -",
                   "BUS_BUSY",
                   "  IDLE",
                   "ARB_LOST",
                   "DATA_ACK",
                   "ADDR_ACK",
                   "  ERROR",
                   "  BUSY"};
  iprintf("SA %02x\n", I2CM->sa);
  iprintf("SR %02x\n", I2CM->sr);
  iprintf("BUF %02x\n", I2CM->buf);
  iprintf("TP  %02x\n", I2CM->tp);
  iprintf("IE  %02x\n", I2CM->ie);
  for (int i = 0; i < 8; i++)
    iprintf("%-8s ", names[i]);
  iprintf("\n");
  uint8_t s = I2CM->sr;
  for (int i = 0x80; i; i >>= 1)
    iprintf("    %d    ", (s & i) != 0);
  iprintf("\n");
  if (s & ERROR)
    for (;;);
  iprintf("\n");
}
#endif

static void command(uint8_t c)
{
  // iprintf("command %02x\n", c);
  flag = 0;
  I2CM->cr = c;
  if (1) {
    usleep(0);
    while (I2CM->sr & BUSY)
      ;
  } else {
    while (!flag)
      ;
  }
  // status();
}

void ft900_i2cm_activate()
{
  // Enable I2CM function (bit 9)
  *sys_regclkcfg |= (1 << 9);
  // *sys_regclkcfg |= (1 << 8);

  // ft900_gpio_set_direction(46, FT900_GPIO_INPUT);
  // ft900_gpio_set_direction(47, FT900_GPIO_INPUT);

  ft900_pad_set_function(44, FT900_PAD_FUNC_1 | FT900_PAD_PULLUP);   // SCL 
  ft900_pad_set_function(45, FT900_PAD_FUNC_1 | FT900_PAD_PULLUP);   // SDA

  // ft900_pad_set_function(46, FT900_PAD_FUNC_1 | FT900_PAD_PULLUP);
  // ft900_pad_set_function(47, FT900_PAD_FUNC_1 | FT900_PAD_PULLUP);

  // Swap I2CM and I2CS pads
  // *sys_regmsc0cfg_b3 |= (1 << 5);

  attach_interrupt(11, i2c_handler);
  I2CM->ie = 0x40;

  ft900_i2cm_set_speed(100);
  i2cm_address = 0x00;
}

void ft900_i2cm_set_address(uint8_t a)
{
  i2cm_address = a << 1;
}

uint8_t ft900_i2cm_get_address()
{
  return i2cm_address >> 1;
}

void ft900_i2cm_set_speed(int khz)
{
  int period = (FT900_SYSTEM_CLOCK / 1000) / khz;
  if (khz >= 1000)
    command(0x10);  // HS mode
  I2CM->tp = (period - 4) >> 3;
}

void ft900_i2cm_clear()
{
  command(0x41);
}

void ft900_i2cm_start_write()
{
  I2CM->sa = i2cm_address;
  command(0x21);
}

void ft900_i2cm_write(uint8_t b)
{
  I2CM->buf = b;
  command(0x01);
}

void ft900_i2cm_start_read(size_t count)
{
  i2cm_count = count;
  I2CM->sa = i2cm_address | 1;
  command(0x23);
}

uint8_t ft900_i2cm_read()
{
  if (--i2cm_count != 0)
    command(0x09);
  else
    command(0x01);
  return I2CM->buf;
}

void ft900_i2cm_stop()
{
      command(0x04);  // STOP
}

