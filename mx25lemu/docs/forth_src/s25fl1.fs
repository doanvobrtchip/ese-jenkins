start-driver s25fl1

$7fb4 org

            beg[
vc.SS_A5    seq       \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq

vc.SS_Q2    seq        \ M5,4 are (1,0)
vc.SS_QI    seq        \ M3-0 are don't care

vc.SS_QI    seq
vc.SS_QI    seq

vc.SS_QI    seq
vc.SS_QI    seq

            ]fin

: s25-xip    h# 20 h# EB firstread ;

: s25-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp s25-xip

: install
    h# 01 madeby if
        ['] s25-methods describes
        h# 01 df-wcmd                   \ Write status registers 1 and 2
        h# 00 >spi
        h# 02 >spi df-wait              \ Quad enable (QE)
        enter-xip autotune
    then ;

end-driver
