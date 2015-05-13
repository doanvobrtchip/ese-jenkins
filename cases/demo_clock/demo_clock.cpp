#include <stdint.h>

#include "libft900.h"
#include "ft800.h"

volatile uint16_t
  t_ms,   // milliseconds, 0-999
  t_s,    // seconds, 0-59
  t_m,    // minutes. 0-59
  t_h,    // hours, 0-23
  t_d;    // days, 0-65535

static void ticker()
{
  if (++t_ms == 1000) {
    t_ms = 0;
    if (++t_s == 60) {
      t_s = 0;
      if (++t_m == 60) {
        t_m = 0;
        if (++t_h == 24) {
          t_h = 0;
          ++t_d;
        }
      }
    }
  }
}

static void show(int x, int y, const char *label, uint32_t val)
{
  VC.command(COLOR_RGB(240,240,255));
  VC.cmd_text(x, y, 27, OPT_CENTER, label);
  VC.command(COLOR_RGB(255,255,255));
  VC.cmd_number(x, y + 24, 30, OPT_CENTER, val);
}

int main()
{
  ft900_timer_activate();
  // Configure prescaler to output 1MHz clock
  ft900_timer_set_prescaler(FT900_SYSTEM_CLOCK / 1000000);
  // Timer 0: 1kHz continuous, counting up 0-999
  ft900_timer_set_prescaling(0, FT900_ENABLE);
  ft900_timer_set_value(0, 1000);
  ft900_timer_set_mode(0, FT900_TIMER_CONTINUOUS | FT900_TIMER_COUNTUP);
  ft900_timer_set_interrupt(0, ticker);
  ft900_timer_start(0);

  ft900_enable_interrupts();

  VC.begin();
  VC.wr(REG_PCLK_POL, 1);
  VC.wr(REG_PCLK, 5);
  VC.wr(REG_GPIO, 0xff);

  uint32_t frames = 1;
  for (;;) {
    VC.cmd_dlstart();
    VC.waitidle();

    // smooth black to brown
    VC.cmd_gradient(0, 0, 0x000000, 0, 255, 0xaa5500);

    // Analog clock
    VC.cmd_clock(136, 136, 132, 0, t_h, t_m, t_s, t_ms);

    // 60Hz frame counter
    show(360, 40, "Frames", frames++);

    // Show the digital counters
    show(320, 150, "Days", t_d);
    show(400, 150, "Hours", t_h);
    show(320, 220, "Minutes", t_m);
    show(400, 220, "Seconds", t_s);

    VC.command(DISPLAY());
    VC.cmd_swap();
  }
}
