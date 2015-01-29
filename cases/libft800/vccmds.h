void VCClass::cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char* s)
{
  wpstart(4 + 2 + 2 + 2 + 2 + strlen(s) + 1);
  spi32(CMD_TEXT);
  spi16(x);
  spi16(y);
  spi16(font);
  spi16(options);
  spis(s);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + strlen(s) + 1 + 3);
}
void VCClass::cmd_number(int16_t x, int16_t y, int16_t font, uint16_t options, int32_t n)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 4);
  spi32(CMD_NUMBER);
  spi16(x);
  spi16(y);
  spi16(font);
  spi16(options);
  spi32(n);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 4 + 3);
}
void VCClass::cmd_loadidentity()
{
  wpstart(4);
  spi32(CMD_LOADIDENTITY);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_toggle(int16_t x, int16_t y, int16_t w, int16_t font, uint16_t options, uint16_t state, const char* s)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1);
  spi32(CMD_TOGGLE);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(font);
  spi16(options);
  spi16(state);
  spis(s);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1 + 3);
}
void VCClass::cmd_gauge(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_GAUGE);
  spi16(x);
  spi16(y);
  spi16(r);
  spi16(options);
  spi16(major);
  spi16(minor);
  spi16(val);
  spi16(range);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_regread(uint32_t ptr, uint32_t result)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_REGREAD);
  spi32(ptr);
  spi32(result);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_getprops(uint32_t ptr, uint32_t w, uint32_t h)
{
  wpstart(4 + 4 + 4 + 4);
  spi32(CMD_GETPROPS);
  spi32(ptr);
  spi32(w);
  spi32(h);
  __end();
  wpbump(4 + 4 + 4 + 4 + 3);
}
void VCClass::cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num)
{
  wpstart(4 + 4 + 4 + 4);
  spi32(CMD_MEMCPY);
  spi32(dest);
  spi32(src);
  spi32(num);
  __end();
  wpbump(4 + 4 + 4 + 4 + 3);
}
void VCClass::cmd_spinner(int16_t x, int16_t y, uint16_t style, uint16_t scale)
{
  wpstart(4 + 2 + 2 + 2 + 2);
  spi32(CMD_SPINNER);
  spi16(x);
  spi16(y);
  spi16(style);
  spi16(scale);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_bgcolor(uint32_t c)
{
  wpstart(4 + 4);
  spi32(CMD_BGCOLOR);
  spi32(c);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_swap()
{
  wpstart(4);
  spi32(CMD_SWAP);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_inflate(uint32_t ptr)
{
  wpstart(4 + 4);
  spi32(CMD_INFLATE);
  spi32(ptr);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_translate(int32_t tx, int32_t ty)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_TRANSLATE);
  spi32(tx);
  spi32(ty);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_stop()
{
  wpstart(4);
  spi32(CMD_STOP);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_slider(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_SLIDER);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(options);
  spi16(val);
  spi16(range);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_touch_transform(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2, uint16_t result)
{
  wpstart(4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 2);
  spi32(CMD_TOUCH_TRANSFORM);
  spi32(x0);
  spi32(y0);
  spi32(x1);
  spi32(y1);
  spi32(x2);
  spi32(y2);
  spi32(tx0);
  spi32(ty0);
  spi32(tx1);
  spi32(ty1);
  spi32(tx2);
  spi32(ty2);
  spi16(result);
  __end();
  wpbump(4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 3);
}
void VCClass::cmd_interrupt(uint32_t ms)
{
  wpstart(4 + 4);
  spi32(CMD_INTERRUPT);
  spi32(ms);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_fgcolor(uint32_t c)
{
  wpstart(4 + 4);
  spi32(CMD_FGCOLOR);
  spi32(c);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_rotate(int32_t a)
{
  wpstart(4 + 4);
  spi32(CMD_ROTATE);
  spi32(a);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char* s)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1);
  spi32(CMD_BUTTON);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(font);
  spi16(options);
  spis(s);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1 + 3);
}
void VCClass::cmd_memwrite(uint32_t ptr, uint32_t num)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_MEMWRITE);
  spi32(ptr);
  spi32(num);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_scrollbar(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_SCROLLBAR);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(options);
  spi16(val);
  spi16(size);
  spi16(range);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_getmatrix(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f)
{
  wpstart(4 + 4 + 4 + 4 + 4 + 4 + 4);
  spi32(CMD_GETMATRIX);
  spi32(a);
  spi32(b);
  spi32(c);
  spi32(d);
  spi32(e);
  spi32(f);
  __end();
  wpbump(4 + 4 + 4 + 4 + 4 + 4 + 4 + 3);
}
void VCClass::cmd_sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t ptr, uint16_t format)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 4 + 2);
  spi32(CMD_SKETCH);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi32(ptr);
  spi16(format);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 4 + 2 + 3);
}
void VCClass::cmd_memset(uint32_t ptr, uint32_t value, uint32_t num)
{
  wpstart(4 + 4 + 4 + 4);
  spi32(CMD_MEMSET);
  spi32(ptr);
  spi32(value);
  spi32(num);
  __end();
  wpbump(4 + 4 + 4 + 4 + 3);
}
void VCClass::cmd_gradcolor(uint32_t c)
{
  wpstart(4 + 4);
  spi32(CMD_GRADCOLOR);
  spi32(c);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_bitmap_transform(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2, uint16_t result)
{
  wpstart(4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 2);
  spi32(CMD_BITMAP_TRANSFORM);
  spi32(x0);
  spi32(y0);
  spi32(x1);
  spi32(y1);
  spi32(x2);
  spi32(y2);
  spi32(tx0);
  spi32(ty0);
  spi32(tx1);
  spi32(ty1);
  spi32(tx2);
  spi32(ty2);
  spi16(result);
  __end();
  wpbump(4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 3);
}
void VCClass::cmd_calibrate(uint32_t result)
{
  wpstart(4 + 4);
  spi32(CMD_CALIBRATE);
  spi32(result);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_setfont(uint32_t font, uint32_t ptr)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_SETFONT);
  spi32(font);
  spi32(ptr);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_logo()
{
  wpstart(4);
  spi32(CMD_LOGO);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_append(uint32_t ptr, uint32_t num)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_APPEND);
  spi32(ptr);
  spi32(num);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_memzero(uint32_t ptr, uint32_t num)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_MEMZERO);
  spi32(ptr);
  spi32(num);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_scale(int32_t sx, int32_t sy)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_SCALE);
  spi32(sx);
  spi32(sy);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_clock(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_CLOCK);
  spi16(x);
  spi16(y);
  spi16(r);
  spi16(options);
  spi16(h);
  spi16(m);
  spi16(s);
  spi16(ms);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
{
  wpstart(4 + 2 + 2 + 4 + 2 + 2 + 4);
  spi32(CMD_GRADIENT);
  spi16(x0);
  spi16(y0);
  spi32(rgb0);
  spi16(x1);
  spi16(y1);
  spi32(rgb1);
  __end();
  wpbump(4 + 2 + 2 + 4 + 2 + 2 + 4 + 3);
}
void VCClass::cmd_setmatrix()
{
  wpstart(4);
  spi32(CMD_SETMATRIX);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_track(int16_t x, int16_t y, int16_t w, int16_t h, int16_t tag)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_TRACK);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(tag);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_getptr(uint32_t result)
{
  wpstart(4 + 4);
  spi32(CMD_GETPTR);
  spi32(result);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_progress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_PROGRESS);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(options);
  spi16(val);
  spi16(range);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_coldstart()
{
  wpstart(4);
  spi32(CMD_COLDSTART);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_keys(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, const char* s)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1);
  spi32(CMD_KEYS);
  spi16(x);
  spi16(y);
  spi16(w);
  spi16(h);
  spi16(font);
  spi16(options);
  spis(s);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 2 + strlen(s) + 1 + 3);
}
void VCClass::cmd_dial(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t val)
{
  wpstart(4 + 2 + 2 + 2 + 2 + 2);
  spi32(CMD_DIAL);
  spi16(x);
  spi16(y);
  spi16(r);
  spi16(options);
  spi16(val);
  __end();
  wpbump(4 + 2 + 2 + 2 + 2 + 2 + 3);
}
void VCClass::cmd_loadimage(uint32_t ptr, uint32_t options)
{
  wpstart(4 + 4 + 4);
  spi32(CMD_LOADIMAGE);
  spi32(ptr);
  spi32(options);
  __end();
  wpbump(4 + 4 + 4 + 3);
}
void VCClass::cmd_dlstart()
{
  wpstart(4);
  spi32(CMD_DLSTART);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_snapshot(uint32_t ptr)
{
  wpstart(4 + 4);
  spi32(CMD_SNAPSHOT);
  spi32(ptr);
  __end();
  wpbump(4 + 4 + 3);
}
void VCClass::cmd_screensaver()
{
  wpstart(4);
  spi32(CMD_SCREENSAVER);
  __end();
  wpbump(4 + 3);
}
void VCClass::cmd_memcrc(uint32_t ptr, uint32_t num, uint32_t result)
{
  wpstart(4 + 4 + 4 + 4);
  spi32(CMD_MEMCRC);
  spi32(ptr);
  spi32(num);
  spi32(result);
  __end();
  wpbump(4 + 4 + 4 + 4 + 3);
}
