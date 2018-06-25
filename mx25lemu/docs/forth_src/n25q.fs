start-driver n25q

\ NB: using the datasheet's method for leaving XIP
\ (p.71: "Leaving XIP After a Controller and Memory Reset")
\ by sending 25 clocks of DQ0=1 does not seem to work.
\ Instead run in quad mode.
\
\ The datasheet (and probably the chip too) is a confused
\ mess. Be warned.

$7f66 org

            beg[
vc.SS_A5    seq         \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_Q0    seq         \ dummy 1 (lsb 0 means continue)
vc.SS_Q0    seq         \

vc.SS_QI    seq
vc.SS_QI    seq

vc.SS_QI    seq
vc.SS_QI    seq         \ dummy 6

vc.SS_QI    seq
vc.SS_QI    seq

vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

: seq-32 ;fallthru
            beg[
vc.SS_A7    seq         \ ADDR 8
vc.SS_A6    seq
vc.SS_A5    seq         \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_Q0    seq         \ dummy 1 (lsb 0 means continue)
vc.SS_Q0    seq         \

vc.SS_QI    seq
vc.SS_QI    seq

vc.SS_QI    seq
vc.SS_QI    seq         \ dummy 6

vc.SS_QI    seq
vc.SS_QI    seq

vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

\ To activate XIP requires two steps. First, enable XIP by
\ setting volatile configuration register bit 3 to 0. Next,
\ drive the XIP confirmation bit to 0 during the next FAST
\ READ operation. XIP is then active. (p.69)
\ QUAD INPUT/OUTPUT FAST READ Command – STR
\ (Extended). p.48
: n25q-xip
    h# 61 df-wcmd h# 6f >spi idle           \ enter quad SPI, disable HOLD
    h# 81 df-qwcmd h# a3 >qpi               \ WRITE VOLATILE, 10 dummy, XIP
    h# 00 h# eb qfirstread ;

: n25q-spi
    h# ff df-qcmd                           \ leave XIP
    h# ff >qpi
    h# ff >qpi
    h# ff >qpi
    h# ff >qpi 10dummy

    h# 61 df-qwcmd h# ff >qpi               \ leave quad SPI
    spi ;

: n25q-methods
    2*route ;fallthru
    jmp n25q-wait
    jmp n25q-erase
    jmp jedec-write
    jmp espim-rd
    jmp n25q-spi
    jmp n25q-xip

: install
    h# 20 madeby if
        ['] n25q-methods describes
        large if
            h# b7 df-wcmd                   \ EN4B command
            d# 32 addressing
            ['] seq-32 inblob sequencer
        then
        enter-xip autotune
    then ;

end-driver
