Interrupt
=========

.. highlight:: c

The FT900 has a global interrupt enable flag.
A common pattern is to disable interrupts for some critical section, then restore their previous state::

  int intstate = ft900_disable_interrupts();

  ... critical section

  ft900_restore_interrupts(intstate);

The interrupt flag maintains the following state

=========== ============================================ =========================================================================
State       Default                                      Accessors
=========== ============================================ =========================================================================
flag        disabled                                     :c:func:`ft900_disable_interrupts`
                                                         :c:func:`ft900_enable_interrupts`
                                                         :c:func:`ft900_restore_interrupts`
=========== ============================================ =========================================================================

.. c:function:: int ft900_enable_interrupts()

  Enables interrupts

  :returns: the previous state of the interrupt enable flag. Use with :c:func:`ft900_restore_interrupts`.

.. c:function:: int ft900_disable_interrupts()

  Disables interrupts

  :returns: the previous state of the interrupt enable flag. Use with :c:func:`ft900_restore_interrupts`.

.. c:function:: void ft900_restore_interrupts(int f)

  Restores the interrupt flag to a previous state

  :param f: interrupt state, as returned by :c:func:`ft900_enable_interrupts` or :c:func:`ft900_disable_interrupts`.

