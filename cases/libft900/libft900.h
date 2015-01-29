#ifndef LIBFT900_H
#define LIBFT900_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>

#define FT900_DISABLE 0
#define FT900_ENABLE  1

#define FT900_PAD_FUNC_0      (0 << 6)
#define FT900_PAD_FUNC_1      (1 << 6)
#define FT900_PAD_FUNC_2      (2 << 6)
#define FT900_PAD_FUNC_3      (3 << 6)
#define FT900_PAD_FUNC_MASK   (3 << 6)

#define FT900_PAD_DRIVE_8     (1 << 4)
#define FT900_PAD_DRIVE_12    (2 << 4)
#define FT900_PAD_DRIVE_16    (3 << 4)
#define FT900_PAD_DRIVE_MASK  (3 << 4)

#define FT900_PAD_PULLUP      (1 << 2)
#define FT900_PAD_PULLDOWN    (2 << 2)
#define FT900_PAD_KEEPER      (3 << 2)
#define FT900_PAD_KEEPER_MASK (3 << 4)

#define FT900_PAD_SCHMITT     (1 << 1)

#define FT900_PAD_SLEW_SLOW   (1 << 0)

#define FT900_GPIO_INPUT      0
#define FT900_GPIO_OUTPUT     4

typedef struct ft900_workbuf_struct {
  union {
#ifndef __cplusplus
    const __flash__ void *flash_data;
#endif
    void *data;
  };
  size_t size;
  void (*completion)(struct ft900_workbuf_struct *);
  uint32_t options;

  struct ft900_workbuf_struct* __next;
} ft900_workbuf_t;

void spi_begin(void);
void spi_sel(uint8_t);
void spi_unsel(void);
uint8_t spi_transfer(uint8_t v);
void spi_bulk_write(const uint8_t *, size_t);
int spi_get_speed(void);
void spi_set_speed(int sp);

void attach_interrupt(int, void (*func)());
int ft900_enable_interrupts();
int ft900_disable_interrupts();
void ft900_restore_interrupts(int);

#define FT900_SYSTEM_CLOCK      90000000
#define FT900_TIMER_CONTINUOUS  0x00
#define FT900_TIMER_ONESHOT     0x01
#define FT900_TIMER_COUNTDOWN   0x00
#define FT900_TIMER_COUNTUP     0x10

// Bitfields for ft900_uart_set_line
#define FT900_UART_5BIT         0x00
#define FT900_UART_6BIT         0x01
#define FT900_UART_7BIT         0x02
#define FT900_UART_8BIT         0x03

#define FT900_UART_2STOP        0x04

#define FT900_UART_ODD_PARITY   0x08
#define FT900_UART_EVEN_PARITY  0x18

void ft900_timer_activate();
void ft900_timer_set_prescaler(uint32_t d);
uint32_t ft900_timer_get_value(uint8_t ti);
void ft900_timer_set_prescaling(uint8_t ti, int ps);
int ft900_timer_get_prescaling(uint8_t ti);
void ft900_timer_set_value(uint8_t ti, uint32_t d);
void ft900_timer_set_mode(uint8_t ti, uint8_t mode);
uint8_t ft900_timer_get_mode(uint8_t ti);
void ft900_timer_set_interrupt(uint8_t ti, void (*func)());
void ft900_timer_start(uint8_t ti);
void ft900_timer_stop(uint8_t ti);

void ft900_uart_activate(size_t uart);
void ft900_uart_set_baud(size_t uart, uint32_t baud);
void ft900_uart_set_line(size_t uart, uint8_t lc);
void ft900_uart_transmit(size_t uart, uint8_t b);
void ft900_uart_add(size_t uart, ft900_workbuf_t *wb);

void ft900_dmac_init(void);
int ft900_dmac_input(size_t *blen, uint8_t *buf);
int ft900_dmac_output(size_t blen, uint8_t *buf);
uint16_t ft900_dmac_mii_read(uint8_t reg);
void ft900_dmac_mii_write(uint8_t reg, uint16_t v);

void ft900_pm_write(uint32_t dst, const void *src, size_t s);
void ft900_pm_read(void *dst, uint32_t src, size_t s);

uint8_t ft900_pad_get_function(uint8_t);
void ft900_pad_set_function(uint8_t, uint8_t);

uint8_t ft900_gpio_get_direction(uint8_t);
void ft900_gpio_set_direction(uint8_t, uint8_t);
void ft900_gpio_write(uint8_t, uint8_t);
uint8_t ft900_gpio_read(uint8_t);

#define FT900_WB_WRITE    0x01
#define FT900_WB_READ     0x02
#define FT900_WB_FLASH    0x04

#define FT900_SPI_SS0   ((1 << 0) ^ 0xff)
#define FT900_SPI_SS1   ((1 << 1) ^ 0xff)
#define FT900_SPI_SS2   ((1 << 2) ^ 0xff)
#define FT900_SPI_SS3   ((1 << 3) ^ 0xff)
#define FT900_SPI_SS4   ((1 << 4) ^ 0xff)
#define FT900_SPI_SS5   ((1 << 5) ^ 0xff)
#define FT900_SPI_SS6   ((1 << 6) ^ 0xff)
#define FT900_SPI_SS7   ((1 << 7) ^ 0xff)

void ft900_spim_add(ft900_workbuf_t *wb);
int ft900_spim_is_idle();
int ft900_spim_get_speed(void);
void ft900_spim_activate();
void ft900_spim_set_speed(int sp);
void ft900_spim_sel(uint8_t s);
void ft900_spim_unsel(void);
uint8_t ft900_spim_transfer(uint8_t v);
// void ft900_spim_add2(ft900_workbuf_t *wb0, ft900_workbuf_t *wb1);

void ft900_dcap_activate(void);
int ft900_dcap_is_idle(void);
void ft900_dcap_add(ft900_workbuf_t *wb);
void ft900_dcap_set_clock_polarity(uint32_t v);
void ft900_dcap_set_trigpat(uint32_t v);
void ft900_dcap_set_count(uint32_t v);
void ft900_dcap_drain();

void ft900_i2cm_activate();
void ft900_i2cm_set_address(uint8_t a);
uint8_t ft900_i2cm_get_address();
void ft900_i2cm_set_speed(int khz);
void ft900_i2cm_clear();
void ft900_i2cm_start_write();
void ft900_i2cm_write(uint8_t b);
void ft900_i2cm_start_read(size_t count);
uint8_t ft900_i2cm_read();
void ft900_i2cm_stop();

void ft900_sdhost_activate();
int ft900_sdhost_card_init();
void ft900_sdhost_read(void *buff, uint32_t sector, uint32_t count);
void ft900_sdhost_set_buswidth(int);
int ft900_sdhost_get_buswidth();

void ft900_i2sm_activate();
void ft900_i2sm_set_padding();
void ft900_i2sm_set_format();
void ft900_i2sm_set_bclks_per_channel();
void ft900_i2sm_set_mclk_source();
void ft900_i2sm_set_bclk_polarity();
void ft900_i2sm_set_lrclk_polarity();
void ft900_i2sm_set_mclk_divider();
void ft900_i2sm_set_bclk_divider();
void ft900_i2sm_add(ft900_workbuf_t *wb);

#ifdef __cplusplus
}
#endif

#endif
