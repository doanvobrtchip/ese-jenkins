Program Memory
==============

.. highlight:: c

The FT900 can both read and write its own program memory.

.. c:function:: void ft900_pm_read(void *dst, uint32_t src, size_t s)

  Read from program memory

  :param dst: pointer to destination area in RAM
  :param src: program memory address
  :param s: byte count, must be multiple of 4

  :returns: none

.. c:function:: void ft900_pm_write(uint32_t dst, const void *src, size_t s)

  Write to program memory

  :param dst: program memory address
  :param src: pointer to source data in RAM
  :param s: byte count, must be multiple of 4

  :returns: none
