start-driver w25q

\ Uses 'Octal Word Read Quad I/O with “Continuous Read Mode”'
\ described on p.47 of datasheet

$7fb4 org

            beg[
vc.SS_A5    seq       \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QA    seq       \ A5 means continue
vc.SS_Q5    seq
            ]fin

: wb-xip    h# A5 h# E3 firstread ;

: wb-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp wb-xip

: install
    h# ef madeby if
        ['] wb-methods describes
        h# 01 df-wcmd                   \ Write status registers 1 and 2
        h# 00 >spi
        h# 02 >spi df-wait              \ Quad enable (QE)
        enter-xip autotune
    then ;

end-driver
