#undef NDEBUG
#include <assert.h>

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <Python.h>
#include "structmember.h"
#include <algorithm>

using namespace std;

#define PRODUCT64(a, b) ((int64_t)(int32_t)(a) * (int64_t)(int32_t)(b))
#define isFF(x) (((x) & 0xff) == 0xff)

#include "vc.h"
#include "topconnect.h"

static uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t crc_update(uint32_t crc, uint8_t data)
{
    int tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
    return crc;
}

uint32_t crc32(uint32_t crc, uint32_t data)
{
  crc = crc_update(crc, 0xff & (data >> 0));
  crc = crc_update(crc, 0xff & (data >> 8));
  crc = crc_update(crc, 0xff & (data >> 16));
  crc = crc_update(crc, 0xff & (data >> 24));
  return crc;
}

typedef int16_t idct_t;

#define SRC(i)    src[(i) * stride]
#define DST(i)    dst[(i) * stride]

static void idct8(idct_t *dst,
                  idct_t *src, size_t stride)
{
#if 0
#define G(X)    (int32_t)((X) * 65536.0)
#define GMUL(a, b)  (PRODUCT64((a), (b)) >> 16)
  idct_t s, t, tmp10, tmp11, tmp12, tmp13, tmp2, tmp3, z1, z2, z3, z4;
  idct_t u, tmp0, tmp1;

  s = GMUL((SRC(2) + SRC(6)), G(0.541196100));

  t = GMUL((SRC(0) + SRC(4)), G(1.0));

  tmp3 = s + GMUL(SRC(2), G(0.765366865));

  tmp10 = t + tmp3;
  tmp13 = t - tmp3;

  t = GMUL((SRC(0) - SRC(4)), G(1.0));
  tmp2 = s + GMUL(SRC(6), G(-1.847759065));

  tmp11 = t + tmp2;
  tmp12 = t - tmp2;

  z3 = SRC(7) + SRC(3);
  z4 = SRC(5) + SRC(1);
  u = GMUL((z3 + z4), G(1.175875602));
  z3 = GMUL(z3, G(-1.961570560)) + u;
  z4 = GMUL(z4, G(-0.390180644)) + u;

  z1 = GMUL((SRC(7) + SRC(1)), G(-0.899976223));
  tmp0 = GMUL(SRC(7), G(0.298631336)) + z1 + z3;
  tmp3 = GMUL(SRC(1), G(1.501321110)) + z1 + z4;

  z2 = GMUL((SRC(5) + SRC(3)), G(-2.562915447));
  tmp1 = GMUL(SRC(5), G(2.053119869)) + z2 + z4;
  tmp2 = GMUL(SRC(3), G(3.072711026)) + z2 + z3;

  DST(0) = (tmp10 + tmp3);
  DST(1) = (tmp11 + tmp2);
  DST(2) = (tmp12 + tmp1);
  DST(3) = (tmp13 + tmp0);
  DST(4) = (tmp13 - tmp0);
  DST(5) = (tmp12 - tmp1);
  DST(6) = (tmp11 - tmp2);
  DST(7) = (tmp10 - tmp3);
#else

  idct_t r[8];
  r[0] = ( (256*SRC(0)) + (355*SRC(1)) + (334*SRC(2)) + (301*SRC(3)) + (256*SRC(4)) + (201*SRC(5)) + (138*SRC(6)) + (70*SRC(7)) ) >> 8;
  r[1] = ( (256*SRC(0)) + (301*SRC(1)) + (138*SRC(2)) + (-70*SRC(3)) + (-256*SRC(4)) + (-355*SRC(5)) + (-334*SRC(6)) + (-201*SRC(7)) ) >> 8;
  r[2] = ( (256*SRC(0)) + (201*SRC(1)) + (-138*SRC(2)) + (-355*SRC(3)) + (-256*SRC(4)) + (70*SRC(5)) + (334*SRC(6)) + (301*SRC(7)) ) >> 8;
  r[3] = ( (256*SRC(0)) + (70*SRC(1)) + (-334*SRC(2)) + (-201*SRC(3)) + (256*SRC(4)) + (301*SRC(5)) + (-138*SRC(6)) + (-355*SRC(7)) ) >> 8;
  r[4] = ( (256*SRC(0)) + (-70*SRC(1)) + (-334*SRC(2)) + (201*SRC(3)) + (256*SRC(4)) + (-301*SRC(5)) + (-138*SRC(6)) + (355*SRC(7)) ) >> 8;
  r[5] = ( (256*SRC(0)) + (-201*SRC(1)) + (-138*SRC(2)) + (355*SRC(3)) + (-256*SRC(4)) + (-70*SRC(5)) + (334*SRC(6)) + (-301*SRC(7)) ) >> 8;
  r[6] = ( (256*SRC(0)) + (-301*SRC(1)) + (138*SRC(2)) + (70*SRC(3)) + (-256*SRC(4)) + (355*SRC(5)) + (-334*SRC(6)) + (201*SRC(7)) ) >> 8;
  r[7] = ( (256*SRC(0)) + (-355*SRC(1)) + (334*SRC(2)) + (-301*SRC(3)) + (256*SRC(4)) + (-201*SRC(5)) + (138*SRC(6)) + (-70*SRC(7)) ) >> 8;

  DST(0) = r[0];
  DST(1) = r[1];
  DST(2) = r[2];
  DST(3) = r[3];
  DST(4) = r[4];
  DST(5) = r[5];
  DST(6) = r[6];
  DST(7) = r[7];

#endif
}


int sx[4] = { 0, 1, -2, -1 }; /* 2-bit sign extension */

int addsat(int16_t x, int16_t y)
{
  int s = x + y;
  if (s < 0)
    return 0;
  else if (s > 255)
    return 255;
  else
    return s;
}

// g# 1.772       0001c5a1
// g# -0.34414    ffffa7e7
// g# -0.71414    ffff492f
// g# 1.402       000166e9


class Ejpg {
private:
  size_t nbits;
  uint64_t bits;

  int state;
  uint8_t store[6 * 64];
  unsigned int bx, by;
  FILE *stim;
  size_t idct_i;
  int idct_phase;
  idct_t aa[64];
  size_t block;
  size_t blk_type;
  int16_t prevdc[4];
  uint16_t restart_i;
  int expect_ff;

  uint16_t rgb565(uint8_t y, int8_t Cb, int8_t Cr) {
    int16_t r, g, b;
#if 0
    r = addsat(GMUL(Cr, G(1.402)), y);
    g = addsat(GMUL(G(-0.34414), Cb) + GMUL(G(-0.71414), Cr), y);
    b = addsat(GMUL(G(1.772), Cb), y);
#else
    r = addsat((           Cr *  359) >> 8, y);
    g = addsat((Cb * -88 + Cr * -183) >> 8, y);
    b = addsat((Cb * 454            ) >> 8, y);
#endif
    uint16_t pix = ((r >> 3) << 11) |
                   ((g >> 2) << 5) |
                   (b >> 3);
    return pix;
  }

public:
  void reset() {
    state = 0;
    // printf("%08x\n", G(1.772));
    // printf("%08x\n", G(-0.34414));
    // printf("%08x\n", G(-0.71414));
    // printf("%08x\n", G(1.402   ));
    stim = NULL;
    // stim = fopen("stim", "w");
  
    bx = 0;
    by = 0;
  }
  void startblock() {
    idct_i = 0;
    idct_phase = 0;
    memset(aa, 0, sizeof(aa));
  }
  void begin() {
    nbits = 0;
    bits = 0;
    bx = 0;
    by = 0;
    block = 0;
    restart_i = 0;
    expect_ff = 0;
    startblock();
    memset(prevdc, 0, sizeof(prevdc));
    // printf("----------------------\n");
  }
  void run2(uint8_t *memory8,
           uint16_t *memory16,
           uint32_t options,
           uint32_t dst,
           uint32_t w,
           uint32_t h,
           uint32_t format,
           uint32_t ri,
           uint32_t tq,
           uint32_t tda,
           uint32_t feed) {

    if (stim)
      fprintf(stim, "%08x\n", feed);

    int bs;
    if (format == 0x11112200)
      bs = 6 * 64;
    else if (format == 0x11111100)
      bs = 3 * 64;
    else if (format == 0x11112100)
      bs = 4 * 64;
    else if (format == 0x00001100)
      bs = 1 * 64;
    else
      assert(0);
    // printf("format=%08x state=%d bs=%d\n", format, state, bs);
      
    int sfeed = (int32_t)feed >> 3;
    if (sfeed < -128)
      feed = 0;
    else if (sfeed > 127)
      feed = 255;
    else
      feed  = sfeed + 128;

    if (0 <= state && (state < bs))
      store[state++] = feed;
    if (state == bs) {

      unsigned int bw, bh;
      uint32_t ex, ey;
      uint8_t y = 0;
      int8_t Cb = 0, Cr = 0;

      switch (format) {

      case 0x11112200:
        bw = 16;
        bh = 16;
        break;

      case 0x11112100:
        bw = 16;
        bh = 8;
        break;

      case 0x11111100:
      case 0x00001100:
        bw = 8;
        bh = 8;
        break;

      default:
        bw = 0;
        bh = 0;
      }

      for (unsigned Y = 0; Y < bh; Y++)
        for (unsigned X = 0; X < bw; X++) {
          switch (format) {

          case 0x11112200: {
            int YB = (X >> 3) + 2 * (Y >> 3);
            int cX = X >> 1, cY = Y >> 1;
            y = store[64 * YB + 8 * (Y & 7) + (X & 7)];
            Cb = store[4 * 64 + 8 * cY + cX] - 128;
            Cr = store[5 * 64 + 8 * cY + cX] - 128;
            break;
          }

          case 0x11112100: {
            int YB = (X >> 3);
            int cX = X >> 1;
            y = store[64 * YB + 8 * (Y & 7) + (X & 7)];
            Cb = store[2 * 64 + 8 * Y + cX] - 128;
            Cr = store[3 * 64 + 8 * Y + cX] - 128;
            break;
          }

          case 0x11111100:
            y = store[8 * (Y & 7) + (X & 7)];
            Cb = store[1 * 64 + 8 * Y + X] - 128;
            Cr = store[2 * 64 + 8 * Y + X] - 128;
            break;

          case 0x00001100:
            y = store[8 * (Y & 7) + (X & 7)];
            Cb = 0;
            Cr = 0;
            break;

          }

          ex = (bw * bx + X);
          ey = (bh * by + Y);

          if ((ex < w) && (ey < h)) {
            if (options & OPT_MONO) {
              uint32_t a = dst + (ey * w + ex);
              memory8[a] = y;
            } else {
              uint32_t a = dst + 2 * (ey * w + ex);
              memory16[a >> 1] = rgb565(y, Cb, Cr);
            }
          }
      }

      state = 0;
      bx++;
      if (bx == ((w + (bw - 1)) / bw)) {
        bx = 0;
        by++;
        if (by == ((h + (bh - 1)) / bh)) {
          // printf("--- finished ---\n");
          // memory8[REG_EJPG_DAT] = 1;
        }
      }
      // printf("bx=%d by=%d\n", bx, by);
    }
  }

  void idct() {
    size_t i;
    idct_t xp[64];

    for (i = 0; i < 8; i++) {
      idct8(xp + 8 * i, aa + 8 * i, 1);
    }
    for (i = 0; i < 8; i++) {
      idct8(aa + i, xp + i, 8);
    }
  }
  void run(uint8_t *memory8,
           uint16_t *memory16,
           uint32_t options,
           uint32_t dst,
           uint32_t w,
           uint32_t h,
           uint32_t format,
           uint32_t ri,
           uint32_t qt,
           uint32_t tda,
           uint8_t  q[2*64],
           int bitsize,
           uint32_t feed) {
    assert((bitsize == 8) | (bitsize == 32));

    const char *btypes = NULL;
    if (format == 0x11112200)
      btypes = "111123";
    else if (format == 0x11111100)
      btypes = "123";
    else if (format == 0x11112100)
      btypes = "1123";
    else if (format == 0x00001100)
      btypes = "1";
    assert(btypes != NULL);

    memory8[REG_EJPG_DAT] = 0;  // not finished yet
    if (bitsize == 8) {
      if (feed == 0xff) {
        expect_ff = 1;
        return;
      }
      if (expect_ff) {
        expect_ff = 0;
        if (feed == 0x00)
          feed = 0xff;
        else {
          // restart marker, next segment, etc. So ignore
          // printf("DISCARD %x\n", feed & 0xff);
          return;
        }
      }
      bits |= ((uint64_t)(feed & 0xff) << (56 - nbits));
      nbits += 8;
    } else {
      assert(!expect_ff);
      assert(!isFF(feed));
      assert(!isFF(feed >> 8));
      assert(!isFF(feed >> 16));
      assert(!isFF(feed >> 24));

      bits |= ((uint64_t)(feed & 0xff) << (56 - nbits));
      nbits += 8;
      bits |= ((uint64_t)((feed >> 8) & 0xff) << (56 - nbits));
      nbits += 8;
      bits |= ((uint64_t)((feed >> 16) & 0xff) << (56 - nbits));
      nbits += 8;
      bits |= ((uint64_t)((feed >> 24) & 0xff) << (56 - nbits));
      nbits += 8;
    }
    assert(nbits <= 64);

    while (1) {
      blk_type = btypes[block] - '0';

      // printf("\n%2d: bits=%016llx\n", nbits, bits);

      uint8_t da8 = tda >> (8 * blk_type);  // DC in bit 4, AC in bit 0
      // printf("blk_type = %d %08x %02x\n", blk_type, tda, da8);
      size_t ht;
      int isdc = idct_i == 0;
      if (isdc)
        ht = REG_EJPG_HT + 64 * (da8 >> 4);
      else
        ht = REG_EJPG_HT + 64 * (2 | (da8 & 1));
      // printf("ht = %x\n", ht);
      uint32_t *pht = (uint32_t*)&memory8[ht];
      size_t codesize;
      uint16_t b = ~0;
      for (codesize = 1; codesize <= min(nbits, (size_t)16); codesize++, pht++) {
        b = bits >> (64 - codesize);
        b &= (1 << codesize) - 1;
        uint16_t hi = *pht >> 16;
        // printf("[%d: b=%x hi=%x] ", codesize, b, hi);
        // printf("%2lu: %08x %d\n", codesize, *pht, b);
        if (b < hi)
          break;
      }
      if (codesize > nbits)
        return; // not enough bits yet

      // have CODESIZE bits in B
      // printf("codesize %lu\n", codesize);
      // printf("symbol at %u\n", 0xff & (b + *pht));
      uint8_t sym_pos = 0xff & (b + *pht);
      size_t symbols;
      if (isdc)
        symbols = REG_EJPG_DCC + 12 * (da8 >> 4);
      else
        symbols = REG_EJPG_ACC + 256 * (da8 & 1);
      uint8_t symbol = memory8[symbols + sym_pos];
      // printf("symbol %02x\n", symbol);

      size_t dmsize = symbol & 0xf;
      if (nbits < (codesize + dmsize))
        return; // not enough bits yet

      assert(dmsize < 12);
      int16_t dm = bits >> (64 - codesize - dmsize);
      dm &= (1 << dmsize) - 1;
      if (dm < (1 << (dmsize - 1)))
        dm = (-(1 << dmsize) ^ dm) + 1;

      // uint32_t consume = bits >> (64 - (codesize + dmsize));
#if 0
      printf("DEBUG: %08x\n", 0x80000000 | symbol);
      if (symbol)
        printf("DEBUG: %08x\n", dm);
#endif
      // printf("%02x,%04x,%02x\n", symbol, 0xffff & dm);

      nbits -= (codesize + dmsize);
      bits <<= (codesize + dmsize);
      run1(memory8,
           memory16,
           options,
           dst,
           w,
           h,
           format,
           ri,
           qt,
           tda,
           q,
           btypes,
           symbol, dm);
    }
    return;
  }
  void run1(uint8_t *memory8,
           uint16_t *memory16,
           uint32_t options,
           uint32_t dst,
           uint32_t w,
           uint32_t h,
           uint32_t format,
           uint32_t ri,
           uint32_t qt,
           uint32_t tda,
           uint8_t  q[2*64],
           const char *btypes,
           uint8_t symbol,
           int16_t dm) {
    // printf("idct_i=%d, symbol %02x\n", idct_i, symbol);
    // printf("SYMBOL %02x,%04x,%02x\n", symbol, 0xffff & dm, idct_i);
    uint8_t runlength = (symbol >> 4) & 15;
    if ((idct_i > 0) && (symbol & 255) == 0) {
      idct_i = 64;
    } else {
      idct_i += runlength;
      size_t qtab = 1 & (qt >> (8 * blk_type));
      uint8_t qi = q[64 * qtab + idct_i];
      // printf("--> %x (q=%d)\n", feed, q[64 * qtab + idct_i]);
      int16_t f = dm;
      if (idct_i == 0) {
        // printf("DC %04x+%04x=%04x\n", 0xffff & prevdc[blk_type], 0xffff & f, 0xffff & (f + prevdc[blk_type]));
        f += prevdc[blk_type];
        prevdc[blk_type] = f;
      }
      int16_t feed = f * qi;

      size_t zz[64] = {0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};
      // printf("Write %04x*%02x=%04x to %02x\n", dm & 0xffff, qi, feed & 0xffff, zz[idct_i]);
      aa[zz[idct_i++]] = feed;
    }

    if (idct_i == 64) {
      // printf("BLOCK\n");
      // printf("%d    %x %s %x %d\n", blk_type, format, btypes, qt, qtab);
      block = (block + 1) % strlen(btypes);
      idct();
      for (size_t i = 0; i < 64; i++)
        run2(memory8,
             memory16,
             options,
             dst,
             w,
             h,
             format,
             ri,
             qt,
             tda,
             aa[i]);
      startblock();
      if (block == 0) {
        if (++restart_i == ri) {
          // printf("RESTART %d\n", ri);
          restart_i = 0;
          memset(prevdc, 0, sizeof(prevdc));

          // printf("gulp %d\n", nbits & 7);
          while (nbits & 7) {
            // printf("\n%2d: bits=%016llx\n", nbits, bits);
            assert(1 & (bits >> 63));
            nbits--;
            bits <<= 1;
          }
        }
      }
    }
  }
};

#define MEMSIZE (4 * 1024 * 1024)
#define REG(N)    memory32[REG_##N >> 2]

class Espim {
private:
  uint32_t count, a;
  int state;
  uint8_t d[64];

public:
  Espim() {
    state = 0;
  }
  int running(void) {
    return state > 0;
  }
  void drive(uint32_t *memory32, uint32_t spimi, uint32_t &spimo, uint32_t &spim_dir, uint32_t &spim_clken) {
    uint64_t *seq = (uint64_t*)&REG(ESPIM_SEQ);

    int si = min(state - 1, 24);

    int opcode;
    if (si < 12)
      opcode = seq[0] >> (5 * si);
    else if (si == 12)
      opcode = ((seq[1] & 1) << 4) + ((seq[0] >> 60) & 0xf);
    else
      opcode = seq[1] >> ((5 * si) & 63);
    opcode &= 0x1f;

    switch (opcode) {
    case SS_PAUSE:  spim_clken = 0; spim_dir = 0x1; spimo = 0;       break;
    case SS_S0:     spim_clken = 1; spim_dir = 0x1; spimo = 0;       break;
    case SS_S1:     spim_clken = 1; spim_dir = 0x1; spimo = 0;       break;
    case SS_A0:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 0;  break;
    case SS_A1:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 4;  break;
    case SS_A2:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 8;  break;
    case SS_A3:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 12; break;
    case SS_A4:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 16; break;
    case SS_A5:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 20; break;
    case SS_A6:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 24; break;
    case SS_A7:     spim_clken = 1; spim_dir = 0xf; spimo = a >> 28; break;
    case SS_QI:     spim_clken = 1; spim_dir = 0x0; spimo = 0;       break;
    default:        spim_clken = 1; spim_dir = 0xf; spimo = opcode;  break;
    }
    spimo &= 0xf;

    uint8_t *window = (uint8_t*)&REG(ESPIM_WINDOW);
    int readstart = REG(ESPIM_READSTART);
    // printf("[%3d]: opcode %x readstart=%d count=%d \n", state, opcode, readstart, count);

    int lastcycle = (state == (readstart + 127));

    d[63] = (spimi) | (d[63] << 4);
    if (!lastcycle)
      state++;
    else {
      memcpy(window, d, 64);
      REG(ESPIM_TRIG) = 1;
      if (--count)
        state = readstart;
      else
        state = 0;
    }
    assert(state < 256);

    if ((state & 1) == (readstart & 1))
      memmove(d, d + 1, 63);

  }
  void trigger(uint32_t *memory32) {
    state = 1;
    a = REG(ESPIM_ADD);
    count = REG(ESPIM_COUNT);
    REG(ESPIM_TRIG) = 0;
  }
};

class vc1_Vc1Object {
public:
  PyObject_HEAD
  /* Type-specific fields go here. */
  bool backdoors;
  uint32_t t;  
  uint32_t d[32]; /* data stack */
  uint32_t r[32]; /* return stack */
  unsigned short pc;    /* program counter, counts CELLS */
  unsigned char dsp, rsp; /* point to top entry */
  uint32_t memrd;
  uint32_t *memory32; /* RAM */
  uint16_t *memory16;
  uint8_t *memory8;
  uint16_t j1boot[16384];
  uint8_t romtop[0x800];        // top 2K of ROM
  uint8_t written[1024];        // read-before-write detector for RAM_J1RAM
  int histo[16384];
  unsigned screenclocks;
  uint32_t touchflip;
  class Ejpg ejpg;
  class Espim espim;
  uint32_t prev_romsub_sel, romsub_sel_[2];
  PyObject *debugs;
  PyObject *pretick;

  uint32_t spimo, spimi;
  uint32_t spim_clken;
  uint32_t spim_dir;
  uint32_t gpo;

  uint32_t sha1key;

  vc1_Vc1Object() {
    pretick = NULL;
  }
  void cpureset()
  {
    pc = 0;
    dsp = rsp = 0;
    t = 0;
  }
  void reset()
  {
    cpureset();
    REG(CPURESET) = 2;
    REG(FREQUENCY) = 48000000;
    REG(CLOCK) = 0;
    ejpg.reset();
    debugs = PyList_New(0);
    memset(written, 0, 1024);
    prev_romsub_sel = 0;
    romsub_sel_[0] = 0;
    romsub_sel_[1] = 0;
  }
  void boot(const char *main_binle)
  {
    FILE *f = fopen(main_binle, "rb");
    if (!f) {
      perror(main_binle);
    }
    size_t n = fread(j1boot, 1, sizeof(j1boot), f);
    assert(n != 0);
    fclose(f);
    memcpy(romtop, j1boot + 0x3c00, 0x800);

    memset(histo, 0, sizeof(histo));

    screenclocks = 0;
    touchflip = 0;
    REG(EJPG_READY) = 1;

    reset();
  }

  void maskregs()
  {
    REG(CMD_READ) &= 0xfff;
    REG(CMD_WRITE) &= 0xfff;
    REG(CMD_DL) &= 0x1fff;
    REG(RAM_FOLD) &= 0x3;
  }

  void compute_space()
  {
    int fullness = 4095 & (REG(CMD_WRITE) - REG(CMD_READ));
    REG(CMDB_SPACE) = (4096 - 4) - fullness;
  }

  void write_checks()
  {
    // ROMSUB simulation 
    uint16_t rs = romsub_sel_[0];

    if (prev_romsub_sel != rs) {
      if (rs == 0)
        memcpy(j1boot + 0x3c00, romtop, 0x800);
      if (rs == 3)
        memcpy(j1boot + 0x3c00, memory8 + RAM_ROMSUB, 0x800);
      prev_romsub_sel = rs;
    }
    romsub_sel_[0] = romsub_sel_[1];
    romsub_sel_[1] = REG(ROMSUB_SEL);
  }

  void push(int v) // push v on the data stack
  {
    dsp = 31 & (dsp + 1);
    d[dsp] = t;
    t = v;
  }

  int pop(void) // pop value from the data stack and return it
  {
    int v = t;
    t = d[dsp];
    dsp = 31 & (dsp - 1);
    return v;
  }

  void selfassert(int p, int line)
  {
    if (!p) {
      printf("Assertion failed at line %d\n", line);
      printf("PC=%04X (%04x) insn %04x\n", pc, pc * 2, j1boot[pc]);
      printf(" T=%08x\n", t);
      printf(" R=");
      for (int i = 0; i < 6; i++)
        printf(" %04X", r[31 & (rsp - i)] / 2);
      printf("\n");
      assert(0);
    }
  }

  PyObject *cycle(int steps)
  {
    uint16_t _pc;
    uint32_t  n, _t;
    int insn = j1boot[pc];
    struct {
      uint16_t pc;
      uint8_t rsp;
    } history[128];
#define HMASK 127

    unsigned screensize = REG(HCYCLE) * REG(VCYCLE) * REG(PCLK);

    if (REG(CPURESET) & 1)
      Py_RETURN_NONE;
    int spichange = 5;

    while (steps--) {
      if ((pretick != NULL) & (spichange > 0)) {
        if (PyObject_CallFunction(pretick, (char*)"O", this) == NULL) {
          return NULL;
        }
        spichange--;
      }
      spim_clken = 0;
      // printf("pc=%04x t=%08x insn=%04x\n", pc, t, insn);
      // selfassert(pc != 0x4bf, __LINE__);
      histo[pc]++;
      insn = j1boot[pc];

      _pc = pc + 1;
      switch (insn >> 13) {
        case 4:
        case 5: // literal
          push(insn & 0x3fff);
          break;
        case 0: // jump
        case 1:
          _pc = insn & 0x3fff;
          break;
        case 7: // conditional jump
          if (pop() == 0)
            _pc = (pc & 0x2000) | (insn & 0x1fff);
          break;
        case 2: // call
        case 3:
          rsp = 31 & (rsp + 1);
          r[rsp] = _pc << 1;
          _pc = insn & 0x3fff;
          break;
        case 6: // ALU
          if (insn & 0x80) { /* R->PC */
            if ((r[rsp] & 1) || (r[rsp] > 32767)) {
              printf("Warning: invalid return address [%d] %08x\n", rsp, r[rsp]);
            }
            _pc = r[rsp] >> 1;
          }
          n = d[dsp];
          switch ((insn >> 8) & 0x1f) {
          case 0x00:  _t = t; break;
          case 0x01:  _t = n; break;
          case 0x02:  _t = t + n; break;
          case 0x03:  _t = t & n; break;
          case 0x04:  _t = t | n; break;
          case 0x05:  _t = t ^ n; break;
          case 0x06:  _t = ~t; break;
          case 0x07:  _t = -(t == n); break;
          case 0x08:  _t = -((int32_t)n < (int32_t)t); break;
          case 0x09:  _t = ((int32_t)(n)) >> t; break;
          case 0x0a:  _t = t - 1; break;
          case 0x0b:  _t = r[rsp]; break;
          case 0x0c:  _t = memrd; break;
          case 0x0d:  _t = t * n; break;
          case 0x0e:  _t = (n << 14) | (t & 0x3fff); break;
          case 0x0f:  _t = -(n < t); break;
          case 0x10:  _t = t + 4;
                      memory32[REG_CRC >> 2] = crc32(memory32[REG_CRC >> 2], memrd); break;
          case 0x11:  _t = n << t; break;
          case 0x12:  _t = memory8[t]; break;
          case 0x13:  assert((t & 1) == 0);
                      _t = memory16[t >> 1]; break;
          case 0x14:  _t = PRODUCT64(t, n) >> 32; break;
          case 0x15:  _t = PRODUCT64(t, n) >> 16; break;
          case 0x16:  _t = (t & ~0xfff) | ((t + 4) & 0xfff); break;
          case 0x17:  _t = n - t; break;
          case 0x18:  _t = t + 1; break;
          case 0x19:  assert((t & 1) == 0);
                      _t = (int32_t)(int16_t)(memory16[t >> 1]); break;
          case 0x1a:  { uint32_t sum32 = t + n; _t = ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); } break;
          case 0x1b:  switch (t >> 12) {
                      case 0: _t = t | RAM_REG; break;
                      case 1: _t = t | RAM_J1RAM; break;
                      default:
                              selfassert(0, __LINE__);
                      }
                      break;
          case 0x1c:  _t = t + 2; break;
          case 0x1d:  _t = t << 1; break;
          case 0x1e:  _t = t + 4; break;
          case 0x1f:  _t = !(isFF(memrd) | isFF(memrd >> 8) | isFF(memrd >> 16) | isFF(memrd >> 24));
                      // printf("xmit %08x\n", memrd);
                      if (_t)
                        ejpg.run(memory8,
                                 memory16,
                                 REG(EJPG_OPTIONS),
                                 REG(EJPG_DST),
                                 REG(EJPG_W),
                                 REG(EJPG_H),
                                 REG(EJPG_FORMAT),
                                 REG(EJPG_RI),
                                 REG(EJPG_TQ),
                                 REG(EJPG_TDA),
                                 &memory8[REG_EJPG_Q],
                                 32,
                                 memrd);
          }
          dsp = 31 & (dsp + sx[insn & 3]);
          rsp = 31 & (rsp + sx[(insn >> 2) & 3]);

          switch (7 & (insn >> 4)) {
          case 0:
            break;
          case 1:
            d[dsp] = t;
            break;
          case 2:
            r[rsp] = t;
            break;
          case 3:
            break;
          case 4:                 // 32-bit write
            selfassert((t & 3) == 0, __LINE__);
            if (t == 0x600000) {
              printf("DEBUG: %08x\n", n);
              PyList_Append(debugs, PyLong_FromUnsignedLong(n));
              break;
            }
            selfassert(t < MEMSIZE, __LINE__);
            selfassert((t < (1024 * 1024)) | (t >= RAM_DL), __LINE__);
            if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
              written[(t - RAM_J1RAM) >> 2] = 1;
            memory32[t >> 2] = n;
            if (t == REG_EJPG_FORMAT) {
              ejpg.begin();
            }
            selfassert(t != REG_EJPG_DAT, __LINE__);
            if (t == REG_J1_INT) {
              REG(INT_FLAGS) |= (n << 5);
            }
            switch (t) {
            case REG_GPIO:
              gpo = n;
              break;
            case REG_SPIM_DIR:
              spichange = 5;
              break;
            case REG_SPIM:
              spichange = 5;
              spim_clken = (n >> 5);
              break;
            case REG_ESPIM_TRIG:
              espim.trigger(memory32);
              break;
            case REG_ESPIM_READSTART:
              selfassert(n < 128, __LINE__);
            }
            break;
          case 5:                 // 16-bit write
            assert((t & 1) == 0);
            assert(t < MEMSIZE);
            assert((t < (1024 * 1024)) | (t >= RAM_DL));
            if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
              written[(t - RAM_J1RAM) >> 2] = 1;
            memory16[t >> 1] = n;
            break;
          case 6:                 // 8-bit write
            selfassert(t < MEMSIZE, __LINE__);
            selfassert((t < (1024 * 1024)) | (t >= RAM_DL), __LINE__);
            memory8[t] = n;
            if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048)))
              written[(t - RAM_J1RAM) >> 2] = 1;
            if (t == REG_EJPG_DAT) {
              ejpg.run(memory8,
                       memory16,
                       REG(EJPG_OPTIONS),
                       REG(EJPG_DST),
                       REG(EJPG_W),
                       REG(EJPG_H),
                       REG(EJPG_FORMAT),
                       REG(EJPG_RI),
                       REG(EJPG_TQ),
                       REG(EJPG_TDA),
                       &memory8[REG_EJPG_Q],
                       8,
                       n);
            }
            break;
          case 7:               // 32-bit read
            selfassert(t < MEMSIZE, __LINE__);
            if ((RAM_J1RAM <= t) && (t < (RAM_J1RAM + 2048))) {
              if (!written[(t - RAM_J1RAM) >> 2])
                printf("address %x\n", t);
              selfassert(written[(t - RAM_J1RAM) >> 2], __LINE__);
            }
            memrd = memory32[t >> 2];
            if (t == REG_SPIM)
              memrd = spimi;
            if (t == REG_ESPIM_TRIG)
              REG(ESPIM_TRIG) = 0;
            break;
          }
          t = _t;
          break;
      }
      pc = _pc;
      maskregs();
      if (memory8[REG_DLSWAP] != 0) {
        memory8[REG_DLSWAP] = 0;
      }

      REG(CLOCK)++;

      if (espim.running()) {
        espim.drive(memory32, spimi, spimo, spim_dir, spim_clken);
        spichange = 5;
      } else {
        spimo = REG(SPIM) & 0x1f;
        spim_dir = REG(SPIM_DIR);
      }

      compute_space();
      write_checks();

      // increment REG_FRAMES for animation
      if (REG(PCLK)) {
        if (++screenclocks > screensize) {
          screenclocks = 0;
          REG(FRAMES)++;
        }
      }
      REG(RASTERY) = (screenclocks > (REG(PCLK) * REG(HCYCLE) * REG(VSIZE))) ? -1 : 0;

      // support touch_fake() scheme
      if (REG(TOUCH_MODE) != touchflip) {
        touchflip = REG(TOUCH_MODE);
        uint32_t xy = ((0xffff & REG(TOUCH_TRANSFORM_A)) << 16) | REG(TOUCH_CHARGE);
        REG(TOUCH_SCREEN_XY) = xy;
        REG(TOUCH_RAW_XY) = xy;
      }
      size_t t0 = memory8[REG_CLOCK] & HMASK;
      history[t0].pc = pc;
      history[t0].rsp = rsp;
      if (pc > 16383) {
        printf("PC escape\n");
        for (size_t i = ((t0 + 1) & HMASK); i != t0; i = (i + 1) & HMASK)
          printf("%04X rsp=%d\n", history[i].pc, history[i].rsp);
        printf("Final PC %04X rsp=%d\n", pc, rsp);
      }
      assert(pc < 16384);
    }
    Py_RETURN_NONE;
  }
};

static int
Vc1_init(vc1_Vc1Object *self, PyObject *args, PyObject *kwds)
{
  self->backdoors = true;
  self->memory32 = new uint32_t[1 << 20];
  memset(self->memory32, 0, 4 << 20);
  self->memory16 = (uint16_t*)self->memory32;
  self->memory8 = (uint8_t*)self->memory32;

  return 0;
}

static void
Vc1_dealloc(vc1_Vc1Object* self)
{
  self->ob_type->tp_free((PyObject*)self);
}

PyObject *vc1_boot(PyObject *self, PyObject *args)
{
  char *main_binle;

  if (!PyArg_ParseTuple(args, "s", &main_binle))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  vc1o->boot(main_binle);
  Py_RETURN_NONE;
}

PyObject *vc1_reset(PyObject *self, PyObject *args)
{
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  vc1o->reset();
  Py_RETURN_NONE;
}

PyObject *vc1_rdstr(PyObject *self, PyObject *args)
{
  unsigned int a, sz;

  if (!PyArg_ParseTuple(args, "II", &a, &sz))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  const char *m = (const char*)vc1o->memory8;

  PyObject *r =  PyString_FromStringAndSize(m + a, sz);

  if (a == REG_INT_FLAGS) {
    vc1o->memory8[REG_INT_FLAGS] = 0;
  }

  return r;
  // return PyString_FromStringAndSize((const char*)r, sz);
}


PyObject *vc1_wrstr(PyObject *self, PyObject *args)
{
  unsigned int a;
  Py_buffer buf;

  if (!PyArg_ParseTuple(args, "Is*", &a, &buf))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  uint8_t *m = (uint8_t*)vc1o->memory8;
  uint32_t *memory32 = vc1o->memory32;
  for (int i = 0; i < buf.len; i++) {
    uint8_t v = ((unsigned char*)(buf.buf))[i];
    if ((a == REG_CPURESET) &&
        ((REG(CPURESET) & 1) == 1) &&
        ((v & 1) == 0)) {
      vc1o->cpureset();
    }
    switch (a) {
    case REG_CMDB_WRITE:
      m[RAM_CMD + (0xffc & REG(CMD_WRITE)) + 0] = v;
      a = REG_CMDB_WRITE + 1;
      break;
    case REG_CMDB_WRITE + 1:
      m[RAM_CMD + (0xffc & REG(CMD_WRITE)) + 1] = v;
      a = REG_CMDB_WRITE + 2;
      break;
    case REG_CMDB_WRITE + 2:
      m[RAM_CMD + (0xffc & REG(CMD_WRITE)) + 2] = v;
      a = REG_CMDB_WRITE + 3;
      break;
    case REG_CMDB_WRITE + 3:
      m[RAM_CMD + (0xffc & REG(CMD_WRITE)) + 3] = v;
      a = REG_CMDB_WRITE + 0;
      REG(CMD_WRITE) = 4095 & (REG(CMD_WRITE) + 4);
      vc1o->compute_space();
      break;
    default:
      m[a++] = v;
      if (a == (RAM_CMD + 4096))
        a = RAM_CMD;
    }
  }
  vc1o->write_checks();
  vc1o->maskregs();
  PyBuffer_Release(&buf);
  Py_RETURN_NONE;
}

PyObject *vc1_sleepclocks(PyObject *self, PyObject *args)
{
  unsigned int a;

  if (!PyArg_ParseTuple(args, "I", &a))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  return vc1o->cycle(a);
}

PyObject *vc1_setpretick(PyObject *self, PyObject *args)
{
  PyObject *pt;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  pt = PyTuple_GET_ITEM(args, 0);

  if (!(PyCallable_Check(pt) || ((pt == Py_None))))
    return PyErr_Format(PyExc_IOError, "pretick argument must be None or callable");

  if (vc1o->pretick != NULL)
    Py_DECREF(vc1o->pretick);

  if (pt == Py_None) {
    vc1o->pretick = NULL;
  } else {
    vc1o->pretick = pt;
    Py_INCREF(pt);
  }
  Py_RETURN_NONE;
}

PyObject *vc1_j1histo(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  PyObject* h = PyList_New(16384);
  for (Py_ssize_t i = 0; i < 16384; i++) {
    PyList_SET_ITEM(h, i, PyLong_FromUnsignedLong(vc1o->histo[i]));
  }
  return h;
}

PyObject *vc1_debugs(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  vc1_Vc1Object* vc1o = ((vc1_Vc1Object*)self);
  Py_INCREF(vc1o->debugs);
  return vc1o->debugs;
}

static PyMethodDef Vc1_methods[] = {
  {"reset", vc1_reset, METH_NOARGS},
  {"boot", vc1_boot, METH_VARARGS},
  {"rdstr", vc1_rdstr, METH_VARARGS},
  {"wrstr", vc1_wrstr, METH_VARARGS},
  {"sleepclocks", vc1_sleepclocks, METH_VARARGS},
  {"j1histo", vc1_j1histo, METH_VARARGS},
  {"debugs", vc1_debugs, METH_VARARGS},
  {"setpretick", vc1_setpretick, METH_VARARGS},
  {NULL}  /* Sentinel */
};

PyObject *get_spimo(vc1_Vc1Object *vc1o, PyObject *args) { return PyInt_FromLong(vc1o->spimo); }
PyObject *get_gpo(vc1_Vc1Object *vc1o, PyObject *args) { return PyInt_FromLong(vc1o->gpo); }
PyObject *get_spim_clken(vc1_Vc1Object *vc1o, PyObject *args) { return PyInt_FromLong(vc1o->spim_clken); }
PyObject *get_spim_dir(vc1_Vc1Object *vc1o, PyObject *args) { return PyInt_FromLong(vc1o->spim_dir); }

#define SETTER(F) \
static int set_##F(vc1_Vc1Object *vc1o, PyObject *value) { \
  long vv = PyInt_AsLong(value); \
  if (PyErr_Occurred()) { \
    return 1; \
  } \
  vc1o->F = vv; \
  return 0; \
}

SETTER(spimi)

static int set_sha1key(vc1_Vc1Object *vc1o, PyObject *value) {
  long vv = PyInt_AsLong(value);
  if (PyErr_Occurred()) {
    return 1;
  }
  vc1o->REG(SHA1KEY) = vv;
  return 0;
}

static PyGetSetDef Vc1_getset[] = {
  {(char*)"gpo",            (getter)get_gpo, (setter)NULL },
  {(char*)"spimo",          (getter)get_spimo, (setter)NULL },
  {(char*)"spim_dir",       (getter)get_spim_dir, (setter)NULL },
  {(char*)"spim_clken",     (getter)get_spim_clken, (setter)NULL },

  {(char*)"spimi",          (getter)NULL, (setter)set_spimi },

  {(char*)"sha1key",        (getter)NULL, (setter)set_sha1key },
  {NULL}
};


static PyMemberDef Vc1_members[] = {
  {NULL}  /* Sentinel */
};

static PyTypeObject vc1_Vc1Type = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "vsimvc1.Vc1",             /*tp_name*/
  sizeof(vc1_Vc1Object), /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)Vc1_dealloc,                         /*tp_dealloc*/
  0,                         /*tp_print*/
  0,               /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
  "Vc1 objects",           /* tp_doc */
  0,                   /* tp_traverse */
  0,                   /* tp_clear */
  0,                   /* tp_richcompare */
  0,                   /* tp_weaklistoffset */
  0,                   /* tp_iter */
  0,                   /* tp_iternext */
  Vc1_methods,             /* tp_methods */
  Vc1_members,             /* tp_members */
  Vc1_getset,                /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Vc1_init,      /* tp_init */
  0,                         /* tp_alloc */
  0,                 /* tp_new */
};

PyObject *randReset(PyObject *self, PyObject *args)
{
  unsigned int vv;

  if (!PyArg_ParseTuple(args, "I", &vv))
    return NULL;

  Py_RETURN_NONE;
}

PyObject *randSeed(PyObject *self, PyObject *args)
{
  unsigned int vv;

  if (!PyArg_ParseTuple(args, "I", &vv))
    return NULL;

  srand48(vv);
  Py_RETURN_NONE;
}

extern "C" void initjsim()
{
  PyObject *m;

  vc1_Vc1Type.tp_new = PyType_GenericNew;
  if (PyType_Ready(&vc1_Vc1Type) < 0)
      return;

  m = Py_InitModule("jsim", NULL);
  Py_INCREF(&vc1_Vc1Type);
  PyModule_AddObject(m, "VC1", (PyObject *)&vc1_Vc1Type);
}
