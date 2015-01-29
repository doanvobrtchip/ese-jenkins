typedef void(*fptr)(void);

extern fptr vector_table[33];

static void nullfunc(void) {}

void attach_interrupt(int intnum, fptr f)
{
  vector_table[intnum] = f;
}

void detach_interrupt(int intnum)
{
  vector_table[intnum] = nullfunc;
}

