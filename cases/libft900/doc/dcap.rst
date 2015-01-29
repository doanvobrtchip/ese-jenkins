DCAP
====

.. highlight:: c

The DCAP (``ft900_dcap``) interface captures streaming data from an external device, for example a digital camera.
The DCAP interface offers flexible triggering options to fit a wide variety of external devices.
Because it is a high-speed interface, its operation is asynchronous, using workbufs.

To initialise the DCAP unit::

  ft900_dcap_activate();

To capture 640 bytes every time HS rises, on the positive edge of the clock::

  ft900_dcap_set_trigpat(0x8);
  ft900_dcap_set_count(640);
  ft900_dcap_set_clock_polarity(1);

The DCAP interface uses workbufs to schedule outstanding work.
Each workbuf represents a read operation from the DCAP device.
For example, to receive 640 bytes from the DCAP interface::

  static void completion_handler(ft900_workbuf_t *wb)
  {
    printf("Received %d bytes\n", wb->size);
  }

  ...

    ft900_workbuf_t wb;
    wb.data = malloc(640);
    wb.size = 640;
    wb.completion = completion_handler;
    ft900_dcap_add(&wb);

The above sequence performs the write asynchronously - the call does not block - and calls ``completion_handler()``
after the data has been received.

The unit maintains the following state

=============== ============================================ =========================================================================
State           Default                                      Accessors
=============== ============================================ =========================================================================
trigger pattern 0x8                                          :c:func:`ft900_dcap_set_trigpat`
count           640                                          :c:func:`ft900_dcap_set_count`
clock polarity  1                                            :c:func:`ft900_dcap_set_clock_polarity`
work queue      Empty                                        :c:func:`ft900_dcap_add` :c:func:`ft900_dcap_is_idle`
=============== ============================================ =========================================================================

.. c:function:: void ft900_dcap_activate()

  Initialise the unit.

.. c:function:: void ft900_dcap_set_trigpat(uint32_t v)

  Sets the trigger pattern

  :param v: trigger pattern 0-15

.. c:function:: void ft900_dcap_set_count(uint32_t v)

  Sets the capture byte count

  :param v: capture count 0-4095

.. c:function:: void ft900_dcap_set_clock_polarity(uint32_t v)

  Sets the clock polarity

  :param v: clock polarity:

     * 1 means that DCAP samples the device data on the rising clock edge
     * 0 means that DCAP samples the device data on the falling clock edge

.. c:function:: void ft900_dcap_add(ft900_workbuf_t *wb)

  Adds a workbuf to the outstanding list of workbufs.
  
  :param wb: The workbuf to append to the module's list of workbufs

  Because DCAP is a read-only interface, 
  :c:member:`ft900_workbuf_t.options`
  is ignored.

  This call returns immediately.
  The read operation is performed in an interrupt handler.

  Note the workbuf ``wb`` is in use until its ``completion`` function is called.

.. c:function:: int ft900_dcap_is_idle(void)

  Returns true if the DCAP unit is idle

  :returns: true if the unit is idle. The unit is idle if there are no outstanding workbufs.

.. c:function:: void ft900_dcap_drain()

  Consumes all data from the DCAP FIFO.
  This function is used when resynchronizing with a streaming device.
