``vc.h`` - main definitions for registers, enumerants and display list commands

``vccmdsdecl.h`` ``vccmds.h`` - command declarations and implementations

``ROM`` - 256K ROM dump, addresses 0xc0000-0xfffff

``dumps`` - dumps of test cases

> Format of the ".vc1dump" file is:
>   sz     field
>   4      version (always decimal 100)
>   4      width of screen in pixels (1-512)
>   4      height of screen in pixels (1-512)
>   4      contents of REG_MACRO_0
>   4      contents of REG_MACRO_1
>   4      Expected CRC32 of image
>   2**18  Main RAM contents
>   2**10  Palette RAM contents
>   2**13  Display list contents
