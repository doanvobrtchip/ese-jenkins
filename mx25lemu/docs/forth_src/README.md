Header information

```
start-driver xxxx             Gives the driver name, same as the file name

$xxxx org                     Set origin address for loader

beg[                          Start of sequencer definition (see below for sequencer ops)
]fin                          End of sequencer definition

end-driver                    Finish driver definition
```

Words in body:

```
h# xx                         Hex constant xx
madeby  ( a -- t/f )          Check if the manufacturer code is a
describes ( a )               Install method table, see below
>spi    ( x )                 Send byte x to SPI
df-cmd  ( x )                 Send command x to SPI
df-wcmd ( x )                 Send write-enable, then command x to SPI
df-wait                       Wait for busy bit to clear
df-ff                         Send command FF (XIP exit)
enter-xip                     Enter XIP mode
autotune                      Adjust sequencer delay register +/-
firstread ( pp cmd )          Do an initial read using SPI cmd, flag byte pp
```

Words for 32-bit address support:

```
large ( -- t/f )              Is capacity >128 Mbit / 16 Mbytes?
addressing ( a )              Set address size to a
inblob sequencer ( a )        install sequencer program from a
```

Sequencer ops:

```
vc.SS_PAUSE seq               No output, no clock
vc.SS_A0-7  seq               Quad output address nibble 0-7
vc.SS_Q0-F  seq               Quad output constant value 0-F
vc.SS_S0-1  seq               Single output constant value 0-1
vc.SS_QI    seq               Quad input
```

Method table:

```
entry   action      default
-----   -----       -------
  0     wait        jedec-wait
  1     erase       jedec-erase
  2     write       jedec-write
  3     read        jedec-rd
  4     leave XIP   (noop)
  5     enter XIP   (noop)
```
