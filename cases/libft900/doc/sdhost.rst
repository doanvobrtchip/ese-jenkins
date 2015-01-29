sdhost
======

.. highlight:: c

To initialise the sdhost unit::

  ft900_sdhost_activate();

the to use an inserted card::

  ft900_sdhost_card_init();

And to read sector zero from the card::

  unsigned char buff[512];
  ft900_sdhost_read(buff, 0, 1);

The unit maintains the following state

=============== ============================================ =========================================================================
State           Default                                      Accessors
=============== ============================================ =========================================================================
buswidth        1                                            :c:func:`ft900_sdhost_set_buswidth`, :c:func:`ft900_sdhost_get_buswidth`
=============== ============================================ =========================================================================

.. c:function:: void ft900_sdhost_activate()

  Initialise the unit.

.. c:function:: int ft900_sdhost_card_init()

  Initialize an sdcard

  :returns: true if card initialization succeeded or zero otherwise

.. c:function:: void ft900_sdhost_set_buswidth(int w)

  Set the bus width used for sdhost-sdcard transfers

  :param w: bus width in bits, either 1 or 4

.. c:function:: int ft900_sdhost_get_buswidth()

  Return the bus width used for sdhost-sdcard transfers

  :returns: bus width in bits, either 1 or 4

.. c:function:: void ft900_sdhost_read(void *buff, uint32_t sector, uint32_t count)

  Read sectors from sdcard into RAM

  :param buff: pointer to destination area, at least ``count`` * 512 bytes.
  :param sector: sector number 0-0xffffffff
  :count: number of sectors to read
