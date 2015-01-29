Pads
====

.. highlight:: c

The FT900 has I/O pins that may be configured for various functions.
At boot, all pads are configured as GPIOs - in order to use a pad with a special-purpose unit, its function must be selected using :c:func:`ft900_pad_set_function`.

A pad's function is controlled by a bitwise OR of a constant from each group:

+-----------+--------------------+------------------------------+
| Group     | Constant           | Meaning                      |
+===========+====================+==============================+
| function  | FT900_PAD_FUNC_0   | Select function 0            |
+           +--------------------+------------------------------+
|           | FT900_PAD_FUNC_1   | Select function 1            |
+           +--------------------+------------------------------+
|           | FT900_PAD_FUNC_2   | Select function 2            |
+           +--------------------+------------------------------+
|           | FT900_PAD_FUNC_3   | Select function 3            |
+-----------+--------------------+------------------------------+
| drive     | FT900_PAD_DRIVE_8  | Drive strength 8 mA          |
+           +--------------------+------------------------------+
|           | FT900_PAD_DRIVE_12 | Select strength 12 mA        |
+           +--------------------+------------------------------+
|           | FT900_PAD_DRIVE_16 | Select strength 16 mA        |
+-----------+--------------------+------------------------------+
| pullup/   | FT900_PAD_PULLUP   | Enable internal 75k pullup   |
+ pulldown  +--------------------+------------------------------+
|           | FT900_PAD_PULLDOWN | Enable internal 75k pulldown |
+           +--------------------+------------------------------+
|           | FT900_PAD_KEEPER   | Enable internal 75k keeper   |
+-----------+--------------------+------------------------------+
| trigger   | FT900_PAD_SCHMITT  | Enable Schmitt trigger input |
+-----------+--------------------+------------------------------+
| slew rate | FT900_PAD_SLOW     | Select slow slew rate        |
+-----------+--------------------+------------------------------+

If no ``drive`` is specified, then the drive strength is 4 mA.

.. c:function:: void ft900_pad_get_function(uint8_t pad)
  
  Return the current function of ``pad``.

  :param pad: pad number 0-65
  :returns: the pad function. See the constant list above.

.. c:function:: void ft900_pad_set_function(uint8_t pad, uint8_t f)

  Set the function of ``pad`` to ``f``.

  :param pad: pad number 0-65
  :param f: the pad function. See the constant list above.
  :returns: None

  :returns: none

