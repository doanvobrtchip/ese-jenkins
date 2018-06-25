start-driver mx25l
$7f8e org

            beg[
vc.SS_A5    seq       \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QA    seq       \ A5 means continue
vc.SS_Q5    seq
vc.SS_QI    seq       \ 4 turnaround cycles
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

: seq-32 ;fallthru
            beg[
vc.SS_A7    seq       \ ADDR 8
vc.SS_A6    seq
vc.SS_A5    seq
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QA    seq       \ A5 means continue
vc.SS_Q5    seq
vc.SS_QI    seq       \ 4 turnaround cycles
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

: mx-xip    h# A5 h# EB firstread ;

: mx-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp mx-xip

: install
    h# c2 madeby if
        ['] mx-methods describes
        h# 01 df-wcmd h# 40 >spi df-wait        \ Quad enable
        large if
            h# b7 df-cmd                        \ EN4B command
            d# 32 addressing
            ['] seq-32 inblob sequencer
        then
        enter-xip autotune
    then ;

end-driver
