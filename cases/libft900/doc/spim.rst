SPI Master
==========

.. highlight:: c

The SPI Master (``ft900_spim``) interface can operate in both a synchronous and asynchronous mode.
The synchronous mode is easier to use.
The asynchronous mode allows high-throughput non-blocking I/O.

To initialise the SPI master unit::

  ft900_spim_activate();

To select the slave connected to SPI select line 0::

  ft900_spim_sel(FT900_SPI_SS0);

To transmit a byte ``0x23`` to the SPI slave::

  ft900_spim_transfer(0x23);

To receive a byte ``r`` from the slave::

  uint8_t r = ft900_spim_transfer(0xff);

To unselect the device, and end the SPI transaction::

  ft900_spim_unsel();

The asynchronous API uses libft900 workbufs to schedule outstanding work.
A workbuf can read or write data from the SPI slave.
In the writing case, the source of the data can be either main RAM or flash memory.
For example, to transmit the string "HELLO WORLD" to an SPI device asynchronously::

  static void completion_handler(ft900_workbuf_t *wb)
  {
    ft900_spim_unsel();
  }

  ...

    char message[] = "HELLO WORLD";
    ft900_workbuf_t wb;
    wb.options = FT900_WB_WRITE;
    wb.data = message;
    wb.size = sizeof(message);
    wb.completion = completion_handler;
    ft900_spim_sel(FT900_SPI_SS0);
    ft900_spim_add(&wb);

The above sequence performs the write asynchronously - the call does not block - and calls ``ft900_spim_unsel()``
after the data has been transmitted.

The unit maintains the following state

=========== ============================================ =========================================================================
State       Default                                      Accessors
=========== ============================================ =========================================================================
speed       8                                            :c:func:`ft900_spim_get_speed` :c:func:`ft900_spim_set_speed`
selects     None selected                                :c:func:`ft900_spim_sel` :c:func:`ft900_spim_unsel`
work queue  Empty                                        :c:func:`ft900_spim_add` :c:func:`ft900_spim_is_idle`
=========== ============================================ =========================================================================

.. c:function:: void ft900_spim_activate()

  Initialise the unit.

.. c:function:: int ft900_spim_get_speed()

  Get the SPI Master speed.

  :returns: the speed divisor ``d``, one of 2, 4, 8, 16, 32, 64, 128, 256, 512.

  The SPI unit divides the system clock by ``d`` to produce a lower-frequency clock for the SPI SCK signal.

.. c:function:: void ft900_spim_set_speed(int d)

  Set the SPI Master speed.

  :param d: SPI Master speed divisor ``d``, one of 2, 4, 8, 16, 32, 64, 128, 256, 512.

  The SPI unit divides the system clock by ``d`` to produce a lower-frequency clock for the SPI SCK signal.
  The slowest possible SPI speed is set by::

    ft900_spim_set_speed(512);

  and the fastest by::

    ft900_spim_set_speed(2);

.. c:function:: void ft900_spim_sel(uint8_t s)

  Assert one of the SPI Master select lines
  
  :param s: one of:
    
    ================= ======
    constant          line
    ================= ======
    ``FT900_SPI_SS0`` 0
    ``FT900_SPI_SS1`` 1
    ``FT900_SPI_SS2`` 2
    ``FT900_SPI_SS3`` 3
    ``FT900_SPI_SS4`` 4
    ``FT900_SPI_SS5`` 5
    ``FT900_SPI_SS6`` 6
    ``FT900_SPI_SS7`` 7
    ================= ======

.. c:function:: void ft900_spim_unsel(void)

  Deassert all SPI Master select lines

.. c:function:: uint8_t ft900_spim_transfer(uint8_t v)

  Transmits a byte over the SPI interface and returns the received byte

  :param v: byte to transmit
  :returns: the byte received over the interface

.. c:function:: void ft900_spim_add(ft900_workbuf_t *wb)

  Adds a workbuf to the outstanding list of workbufs.
  
  :param wb: The workbuf to append to the module's list of workbufs

  The workbuf should specify whether its data is to be read or written by setting :c:member:`ft900_workbuf_t.options`.

     * ``FT900_WB_READ`` means that the workbuf's ``data`` is filled by data received from the SPI slave.
     * ``FT900_WB_WRITE`` means that the workbuf's ``data`` is transmitted to the SPI slave.
     * ``FT900_WB_FLASH`` means that the workbuf's ``data`` is transmitted to the SPI slave, and that the data resides in flash memory.

  This call returns immediately. The SPI operations are performed in an interrupt handler.

  Note the workbuf ``wb`` is in use until its ``completion`` function is called.

.. c:function:: int ft900_spim_is_idle()

  Returns true if the SPI unit is idle

  :returns: true if the unit is idle. The unit is idle if there are no outstanding workbufs.
