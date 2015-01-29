Timer
=====

.. highlight:: c

The timer unit has one 16-bit prescaler, and four independent 16-bit timers.
Each timer can generate an interrupt.

To initialise the timer unit::

  ft900_timer_activate();

to configure the prescaler to produce a 1MHz clock::

  ft900_timer_prescaler_set(FT900_SYSTEM_CLOCK / 1000000);

setting timer 0's limit to 1000::

  ft900_timer_set_prescaling(0, FT900_ENABLE);
  ft900_timer_set_value(0, 1000);
  ft900_timer_set_mode(0, FT900_TIMER_CONTINUOUS | FT900_TIMER_COUNTUP);

to make the timer generate an interrupt when it reaches 1000, first define an interrupt handler, for example::

  volatile unsigned int ms;

  static void tickms()
  {
    counter++;
  }

then set ``tickms`` to be called every time timer 0 reaches its limit::

  ft900_timer_set_interrupt(0, tickms);
  ft900_timer_start(0);

The timer unit maintains the following state

=========== ============================================ =========================================================================
State       Default                                      Accessors
=========== ============================================ =========================================================================
prescaler   0                                            :c:func:`ft900_timer_get_prescaler` :c:func:`ft900_timer_set_prescaler`
prescaling  FT900_DISABLE                                :c:func:`ft900_timer_get_prescaling` :c:func:`ft900_timer_set_prescaling`
mode        FT900_TIMER_CONTINUOUS | FT900_TIMER_COUNTUP :c:func:`ft900_timer_get_mode` :c:func:`ft900_timer_set_mode`
interrupt   NULL                                         :c:func:`ft900_timer_get_interrupt` :c:func:`ft900_timer_set_interrupt`
value       0                                            :c:func:`ft900_timer_get_value` :c:func:`ft900_timer_set_value`
=========== ============================================ =========================================================================

.. c:function:: void ft900_timer_activate()

  Initialise the timer unit.

  :returns: none

.. c:function:: uint32_t ft900_timer_get_prescaler()

  Get the prescaler's divisor value.

  :returns: the prescaler's divisor value ``d`` 2-65536

  The prescaler divides the system clock by ``d`` to produce a lower-frequency clock.
  This clock may be used as an input by any of the four timers, see :c:func:`ft900_timer_set_prescaling()`.

.. c:function:: void ft900_timer_set_prescaler(uint32_t d)

  Set the prescaler's divisor value.

  :param d: prescaler divisor 2-65536
  :returns: none

  The prescaler divides the system clock by ``d`` to produce a lower-frequency clock.
  This clock may be used as an input by any of the four timers, see :c:func:`ft900_timer_set_prescaling()`.


.. c:function:: uint32_t ft900_timer_get_value(uint8_t ti)

  :param ti: timer 0-3
  :returns: the instantaneous value of the timer's counter

.. c:function:: void ft900_timer_set_value(uint8_t ti, uint32_t d)

  Set the timer's counter limit/start value.

  :param ti: timer 0-3
  :param d: counter limit value (FT900_TIMER_COUNTUP) or start value (FT900_TIMER_COUNTDOWN).
  :returns: none

.. c:function:: int ft900_timer_get_prescaling(uint8_t ti)

  Return prescaled input selection for timer ``ti``.

  :param ti: timer 0-3
  :returns: FT900_ENABLE, indicating prescaler input, or FT900_DISABLE indicating main clock input

.. c:function:: void ft900_timer_set_prescaling(uint8_t ti, int ps)

  Select main or prescaled clock input for timer ``ti``.

  :param ti: timer 0-3
  :param ps: select prescaler input (FT900_ENABLE) or main clock input (FT900_DISABLE)
  :returns: none

.. c:function:: uint8_t ft900_timer_get_mode(uint8_t ti)

  :param ti: timer 0-3
  :returns: none

.. c:function:: void ft900_timer_set_mode(uint8_t ti, uint8_t mode)

  :param ti: timer 0-3
  :param mode: a logical OR of either FT900_TIMER_CONTINUOUS or FT900_TIMER_ONESHOT, and 
   either FT900_TIMER_COUNTDOWN or FT900_TIMER_COUNTUP.
  :returns: none

  To set timer 3 for continuous countdown operation::

    ft900_timer_set_mode(3, FT900_TIMER_CONTINUOUS | FT900_TIMER_COUNTDOWN);

.. c:function:: int ft900_timer_get_interrupt(uint8_t ti)

  :param ti: timer 0-3
  :returns: zero if the timer's interrupt is enabled, non-zero otherwise

.. c:function:: void ft900_timer_set_interrupt(uint8_t ti, void (*func)())

  Set the interrupt handler for the timer ``ti``.  If ``func`` is NULL, disable the interrupt handler.

  :param ti: timer 0-3
  :param func: pointer to interrupt handler function, or NULL
  :returns: none

.. c:function:: void ft900_timer_start(uint8_t ti)

  Start timer ``ti``.

  :param ti: timer 0-3
  :returns: none

.. c:function:: void ft900_timer_stop(uint8_t ti)

  Stop timer ``ti``.

  :param ti: timer 0-3
  :returns: none
