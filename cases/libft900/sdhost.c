#include "libft900.h"
#include "libft900internal.h"
 
// Present State register
#define SYS_CARD_INSERT     0x00010000U
#define BUFFER_READ_ENABLE  0x00000800U
#define CMD_INHIBIT_D       0x00000002U
#define CMD_INHIBIT_C       0x00000001U

// Clock control register
#define INTER_CLK_EN        0x00000001U
#define INTER_CLK_STABLE    0x00000002U
#define SD_CLK_EN           0x00000004U

#define VERBOSE 1

#ifdef VERBOSE
static void status()
{
  iprintf("%08lx %08lx\n", *sdhost_presentstate, *sdhost_clockcontrol_timeoutcontrol_softwarereset);
}
#endif

// Command register
#define DATA_PRES_SEL     0x0020U
#define CMD_IDX_CHECK_EN  0x0010U
#define CMD_CRC_CHECK_EN  0x0008U
// See table 4-10
#define R_      0           // For commands that do not expect a response
#define R2      (1 | CMD_CRC_CHECK_EN)
#define R3      (2)
#define R4      (2)
#define R1      (2 | CMD_IDX_CHECK_EN | CMD_CRC_CHECK_EN)
#define R5      (2 | CMD_IDX_CHECK_EN | CMD_CRC_CHECK_EN)
#define R6      (2 | CMD_IDX_CHECK_EN | CMD_CRC_CHECK_EN)
#define R7      (2 | CMD_IDX_CHECK_EN | CMD_CRC_CHECK_EN)
#define R1b5b   (3 | CMD_IDX_CHECK_EN | CMD_CRC_CHECK_EN)  // For response R1b,R5b

static void wait_ready_command()
{
  // Wait for cmd_inhibit_c == 0
  while ((*sdhost_presentstate & CMD_INHIBIT_C) != 0)
    ;

  while ((*sdhost_presentstate & CMD_INHIBIT_D) != 0)
    ;
}

static void wait_int_response()
{
  while ((*sdhost_normintstatus_errintstatus & 1) == 0) {
    /*iprintf("%08lx\n", *sdhost_normintstatus_errintstatus)*/;
  }
  *sdhost_normintstatus_errintstatus = 1;
}

static uint32_t command(uint8_t c, int resp, uint32_t arg)
{
  wait_ready_command();

  uint16_t cmd = (c << 8) | resp | (0 << 5);

  *sdhost_arg1reg = arg;
  *sdhost_tmr_cmdr = (cmd << 16);

#ifdef VERBOSE
  iprintf("command: %04x,%08lx -> ", cmd, *sdhost_arg1reg);
#endif

  wait_int_response();
#ifdef VERBOSE
  iprintf("response: %08lx %08lx %08lx %08lx (%08lx)\n",
    *sdhost_response0,
    *sdhost_response1,
    *sdhost_response2,
    *sdhost_response3, *sdhost_normintstatus_errintstatus);
#endif
  return *sdhost_response0;
}

uint32_t appcmd(uint8_t c, uint32_t arg)
{
  command(55, R1, 0);
  return command(c, R1, arg);
}

#define RESET_ALL (1 << 24)

static void vendor_setup()
{
  volatile uint32_t *vendor = sdhost_vendor1;
  vendor[0] = 0x02000101; // pulse latch offset
  vendor[5] = 0xb;  // Debounce to maximum
#ifdef VERBOSE
  for (int i = 0; i <= 9; i++)
    iprintf("Vendor %d: %08lx\n", i, vendor[i]);
#endif
}

void ft900_sdhost_activate()
{
  *sys_regclkcfg |= (1 << 12);

  for (int i = 19; i <= 26; i++)
    ft900_pad_set_function(i, FT900_PAD_FUNC_1);

  *sdhost_clockcontrol_timeoutcontrol_softwarereset |= RESET_ALL;
  while (*sdhost_clockcontrol_timeoutcontrol_softwarereset & RESET_ALL)
    ;

#ifdef VERBOSE
  iprintf("VERSION=%08lx\n", *sdhost_specversion);
  iprintf("Capabilities: %08lx %08x\n", *sdhost_capreg0, *sdhost_capreg1);
  iprintf("cmd12/hostcontrol2: %08lx\n", *sdhost_autocmd12_hostcontrol2);
#endif
  *sdhost_autocmd12_hostcontrol2 |= (0x8 | 2) << 16;

#ifdef VERBOSE
  iprintf("cmd12/hostcontrol2: %08lx\n", *sdhost_autocmd12_hostcontrol2);
#endif

  vendor_setup();

  // Enable all error and interrupt bits
  *sdhost_normintstatus_en_errintstatus_en = ~0;

#define CLOCK_SLOW 0xffc0
#define CLOCK_FAST 0x0000

  // Enable clock
  *sdhost_clockcontrol_timeoutcontrol_softwarereset |= (INTER_CLK_EN | CLOCK_FAST);

  // Wait until clock is stable
  while (!(*sdhost_clockcontrol_timeoutcontrol_softwarereset & INTER_CLK_STABLE))
    ;
  // Enable clock output
  *sdhost_clockcontrol_timeoutcontrol_softwarereset |= SD_CLK_EN;

}

static struct {
  uint32_t rca;
} card;

int ft900_sdhost_card_init()
{
  command(0x00, R_, 0x00);

  int F8 = command(0x08, R1, 0x1aa) == 0x1aa;
#ifdef VERBOSE
  iprintf("F8=%d\n", F8);
#endif

  uint32_t response;
  while (1) {
    command(55, R1, 0);
    response = command(41, R3, 0x00FF8000 | (F8 ? (1UL << 30) : 0));
    if (response & 0x80000000)
      break;
    usleep(100 * 1000);
  }

  // CMD2: switch from ready state to ident.
  command(2, R2, 0);

  // CMD3: get RCA
  card.rca = 0xffff0000 & command(3, R6, 0);  // get 

  // CSD register
  command(9, R2, card.rca);

  uint32_t C_SIZE = (*sdhost_response1 >> 8) & ((1 << 23) - 1);
#ifdef VERBOSE
  iprintf("C_SIZE = %lx\n", C_SIZE);
  iprintf("capacity = %lu Kbyte\n", (C_SIZE + 1) * 512);
#endif

  // Put card in transfer state
  if (command(7, R1b5b, card.rca) != 0x700) {
#ifdef VERBOSE
    iprintf("Failed to enter transfer state\n");
#endif
    return 0;
  }

  command(16, R1, 512);   // SET_BLOCKLEN

  *sdhost_bsr_bcr = (0x0001 << 16) | 0x200;
#ifdef VERBOSE
  iprintf("Block Size register %08lx\n", *sdhost_bsr_bcr);
  status();
#endif
  *sdhost_normintstatus_errintstatus = 2;

  return 1;
}

void ft900_sdhost_set_buswidth(int w)
{
  int fourbit = (w == 4);

  command(55, R1, card.rca);
  command(6, R1, fourbit ? 0x2 : 0x0);
  *sdhost_hostcontrol1_powercontrol_blockgapcontrol |= (fourbit << 1);
}

int ft900_sdhost_get_buswidth()
{
  int fourbit = (*sdhost_hostcontrol1_powercontrol_blockgapcontrol & 1);
  return fourbit ? 4 : 1;
}

void ft900_sdhost_read(void *buff, uint32_t sector, uint32_t count)
{
  uint16_t trns =
    ( 0 << 2) |   // block_count_en
    ( 1 << 4);    // trns read

  while (count--) {
    uint16_t cmd = (17 << 8) | R1 | (1 << 5);

    wait_ready_command();
    *sdhost_arg1reg = sector++;
    *sdhost_tmr_cmdr = (cmd << 16) | trns;

    // iprintf("command: %04x,%08lx\n", cmd, *sdhost_arg1reg);
    wait_int_response();
    while ((*sdhost_presentstate & BUFFER_READ_ENABLE) == 0)
      ;
    // iprintf("INT %08lx %08lx\n", *sdhost_normintstatus_errintstatus, *sdhost_normintstatus_en_errintstatus_en);

#if 0
    for (int i = 0; i < 0x80; i++)
      buff[i] = *sdhost_bufferdport;
#else
      asm("streamin.l %0,%1,%2" \
          : \
          :"r"(buff), "r"(sdhost_bufferdport), "r"(512));
#endif
      buff += 512;
  }
}


