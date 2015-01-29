Workbufs
========

.. highlight:: c

The asychronous APIs use workbufs to represent outstanding I/O transfers.
The C type is :c:type:`ft900_workbuf_t`.
The user program is responsible for creating and managing workbufs and their associated
memory.

.. c:type:: ft900_workbuf_t

  The workbuf's :c:member:`ft900_workbuf_t.data` specifies the address of the data.
  :c:member:`ft900_workbuf_t.size` specifies the data size, in bytes.

  .. c:member:: data

    Pointer to the workbuf's data in RAM.
    This data may be read, written or both, depending on the operation.

  .. c:member:: const __flash__ void* flash_data

    Pointer to the workbuf's data in flash.
    This data may only be read, so this pointer only makes sense with write-only operations.

  .. c:member:: size_t size

    Size of the workbuf data, in bytes.
    A size of zero is invalid.

  .. c:member:: uint32_t options

    Options controlling the operation:

      ================================ ==========================================================================================
      Option                           Meaning
      ================================ ==========================================================================================
      ``FT900_WB_READ``                the workbuf's ``data`` is filled by data received from a device
      ``FT900_WB_WRITE``               the workbuf's ``data`` is transmitted to the device
      ``FT900_WB_FLASH``               the workbuf's ``data`` is transmitted to the device, and that the data resides in flash memory
      ``FT900_WB_READ|FT900_WB_WRITE`` the workbuf's ``data`` is both transmitted to and relaced by data received from the device
      ================================ ==========================================================================================

  .. c:member:: void (*completion)(ft900_workbuf_t *)

    This function is called after the operation has completed,
    and resources for this workbuf can be freed.
    Note that this function is called inside an interrupt handler.
    ``completion`` may be NULL, in which case no function is called.
