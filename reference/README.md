# ROM to C Header

- /reference/vcXroms/**mainrom.bin**, **ROM**, **rom_mainXX** ➜ /bt8XXemu/resources/rom_bt8XX.h
- /reference/vxXroms/**rom_rcosatan** *(not used)*
- /reference/vxXroms/**rom_sample** *(not used)*
- /reference/vcXroms/**rom_j1boot** ➜ /bt8XXemu/resources/crom_bt8XX.h
- /reference/vxXroms/**rom_jaboot** *(not used)*
- /reference/vxXroms/**rom_jtboot** *(not used)*
- /reference/vxXroms/**rom_jxboot** *(not used)*
- /reference/vcXroms/**otp_8XX.hex** ➜ /bt8XXemu/resources/otp_8XX.h

## Processes

### rom_mainXX

To convert the rom_mainXX ASCII hex files into a binary
- Read two bytes (four characters plus newline) from each of 16 main rom files.
- Interlace them into a single output file.
- Repeat.
For a byte array output, the 2 byte words are read from the hex file
as big endian (human hex notation) and written as little endian (so, flipped).

# Revisions

## BT88x
- 880/881 ➜ RGB666 (QFN48), 882/883 ➜ RGB888(QFN56)
- 881/883 ➜ Capacitive Touch, 880/882 ➜ Resistive Touch

# Reference

``vc.h`` - main definitions for registers, enumerants and display list commands

``vccmdsdecl.h`` ``vccmds.h`` - command declarations and implementations

``ROM`` - 256K ROM dump, addresses 0xc0000-0xfffff

``dumps`` - dumps of test cases

    Format of the ".vc1dump" file is:
      sz     field
      4      version (always decimal 100)
      4      width of screen in pixels (1-512)
      4      height of screen in pixels (1-512)
      4      contents of REG_MACRO_0
      4      contents of REG_MACRO_1
      4      Expected CRC32 of image
      2**18  Main RAM contents
      2**10  Palette RAM contents
      2**13  Display list contents
