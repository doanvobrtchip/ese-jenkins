GPIO
====

.. highlight:: c

All FT900 pads are usable as GPIOs.
A pad's I/O direction is controlled by :c:func:`ft900_gpio_set_direction`.

To produce a 1 KHz square wave on pin 4::

  ft900_gpio_set_direction(4, FT900_GPIO_OUTPUT);
  for (;;) {
    ft900_gpio_write(4, 0);
    usleep(500);
    ft900_gpio_write(4, 1);
    usleep(500);
  }

And to set pin 4 to the inverse of pin 7::

  ft900_gpio_write(4, !ft900_gpio_read(7));

.. c:function:: uint8_t ft900_gpio_get_direction(uint8_t pad)
  
  Return the current direction setting of ``pad``.

  :param pad: pad number 0-65
  :returns: the pad direction, either ``FT900_GPIO_INPUT`` or ``FT900_GPIO_OUTPUT``.

.. c:function:: void ft900_gpio_set_direction(uint8_t pad, uint8_t dir)
  
  Set the direction of ``pad`` to ``dir``.

  :param pad: pad number 0-65
  :param dir: the pad direction, either FT900_GPIO_INPUT or FT900_GPIO_OUTPUT.
  :returns: None

.. c:function:: void ft900_gpio_write(uint8_t pad, uint8_t s)
  
  Set pad to value ``s``.

  :param pad: pad number 0-65
  :param s: the pad value, 0 or 1
  :returns: None

  The pad should previously have been configured as an FT900_GPIO_OUTPUT using :c:func:`ft900_gpio_set_direction`.

.. c:function:: uint8_t ft900_gpio_read(uint8_t pad)
  
  Read the current pad sense.

  :param pad: pad number 0-65
  :returns: the pad value, 0 or 1

  The pad should previously have been configured as an FT900_GPIO_INPUT using :c:func:`ft900_gpio_set_direction`.
