#ifndef FT800_H
#define FT800_H

#include <stdint.h>
#include "vc.h"

#define F16(s)  ((int32_t)(s * 65536L))

class VCClass {
public:
  void begin();
  static void end();
  static void __start(uint32_t addr);
  static void __wstart(uint32_t addr);
  static void __end(void);
  static uint8_t rd(uint32_t addr);
  static void wr(uint32_t addr, uint8_t v);
  static unsigned int rd16(uint32_t addr);
  static void wr16(uint32_t addr, unsigned int v);
  static void wr32(uint32_t addr, unsigned long v);
  static void spi32(uint32_t v);
  static uint32_t rd32(uint32_t addr);
  static void fill(uint32_t addr, uint8_t v, uint32_t count);
  void command(uint32_t c);
  void waitidle();

  static void dlstart();
  static void dl(unsigned long cmd);
  static void dlend();

  void wpstart(size_t a);
  void getfree();
  void wpbump(uint16_t a);
  void copy(uint32_t addr, const uint8_t *src, size_t count);
  void copycmds(const uint32_t *src, size_t count);
  void dumpscreen(bool wait = true);

  #include "vccmdsdecl.h"

  uint16_t wp;
  uint16_t freespace;
};
extern VCClass VC;

#endif
