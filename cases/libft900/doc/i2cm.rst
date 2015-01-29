I2C Master
==========

.. highlight:: c

The I2C Master (``ft900_i2c``) interface can operate in both a synchronous and asynchronous mode.
The synchronous mode is easier to use.
The asynchronous mode allows high-throughput non-blocking I/O.

To initialise the I2C master unit::

  ft900_i2cm_activate();

To read the time (hours, minutes, seconds) from a DS1307 real-time clock (RTC)::

  ft900_i2cm_set_address(0xd0 >> 1);
  // Address register 0x00
  ft900_i2cm_start_write();
  ft900_i2cm_write(0x00);
  // Read 3 bytes
  uint8_t s,m,h;
  ft900_i2cm_start_read(3);
  s = ft900_i2cm_read();
  m = ft900_i2cm_read();
  h = ft900_i2cm_read();
  // STOP
  ft900_i2cm_stop();

The asynchronous API uses libft900 workbufs to schedule outstanding work.
A workbuf can read or write data from the I2C slave.
In the writing case, the source of the data can be either main RAM or flash memory.

The unit maintains the following state

=========== ============================================ =========================================================================
State       Default                                      Accessors
=========== ============================================ =========================================================================
address     0x00                                         :c:func:`ft900_i2cm_get_address` :c:func:`ft900_i2cm_set_address`
speed       100 KHz                                      :c:func:`ft900_i2cm_set_speed`
work queue  Empty                                        :c:func:`ft900_i2cm_add` :c:func:`ft900_i2cm_is_idle`
=========== ============================================ =========================================================================

.. c:function:: void ft900_i2cm_activate()

  Initialise the unit.

.. c:function:: void ft900_i2cm_set_speed(int khz)

  Set the I2C Master speed

  :param khz: Requested I2C Master speed, in KHz.

  The I2C unit uses a frequency divisor to approximate the requested speed. To set up 100 KHz I2C, use::

    ft900_i2cm_set_speed(100);

  and for 400 kHz I2C::

    ft900_i2cm_set_speed(400);

.. c:function:: void ft900_i2cm_set_address(uint8_t a)

  Set the 7-bit slave address

.. c:function:: uint8_t ft900_i2cm_get_address()

  Get the 7-bit slave address

  :returns: the 7-bit slave address

.. c:function:: void ft900_i2cm_clear()

  Clear any slaves on the I2C bus by generating 9 clocks followed by STOP

.. c:function:: void ft900_i2cm_start_write()

  Send a bus START condition followed by the slave address.

.. c:function:: void ft900_i2cm_write(uint8_t b)

  Write a single byte to the I2C slave.

  :param b: The byte value to transmit to the I2C slave.

.. c:function:: void ft900_i2cm_start_read(size_t count)

  Send a bus START condition followed by the slave address.

  :param count: number of bytes that will be read using :c:func:`ft900_i2cm_read`.

.. c:function:: uint8_t ft900_i2cm_read()

  Read one byte from the I2C slave.

  :returns: the byte read from the I2C slave

.. c:function:: void ft900_i2cm_stop()

  Send the STOP condition.
