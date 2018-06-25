start-driver unified

$7ee8 org

            beg[
vc.SS_A5    seq       \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QA    seq       \ A5 means continue
vc.SS_Q5    seq

vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq

            ]fin

: f-eb-xip
    h# 40 df-setstatus                      \ Set QE bit
    df-wait
    eb-xip ;

: f-eb-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp f-eb-xip

\ N25Q needs multiple die erase commands

: n25q-erase ( -- )
    leave-xip
    vc.REG_FLASH_SIZE @ d# 128 >= if        \ erase each die
        vc.REG_FLASH_SIZE @ d# 0            \ count up from 0 in 64 MB
        begin
            dup d# 20 lshift dieerase
            d# 64 + 2dup=
        until 2drop
    else
        h# c7 df-wcmd                       \ Bulk erase
    then
    n25q-wait
    enter-xip ;

: war-n25q-methods
    2*route ;fallthru
    jmp n25q-wait
    jmp n25q-erase
    jmp jedec-write
    jmp espim-rd
    jmp rom-n25q-spi
    jmp rom-n25q-xip

\ W25M lacks true XIP

: seq-w25m ;fallthru
            beg[
vc.SS_S1    seq       \ E
vc.SS_S1    seq
vc.SS_S1    seq
vc.SS_S0    seq
vc.SS_S1    seq       \ C
vc.SS_S1    seq
vc.SS_S0    seq
vc.SS_S0    seq

vc.SS_A7    seq
vc.SS_A6    seq
vc.SS_A5    seq
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QI    seq       \ 6 turnaround cycles
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

: w25m-xip
    spi idle ;

: w25m-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp (noop)
    jmp w25m-xip

: install
    jedec-manufacturer
    h# c8 over= if                          \ GD25Q
        ['] eb-methods
        h# 31 df-wcmd h# 02 >spi
        jedec-inst exit
    then
    h# 9d over= if                          \ IS25L
        ['] eb-methods
        h# 40 df-setstatus
        jedec-inst exit
    then
    h# c2 over= if                          \ MX25
        ['] f-eb-methods jedec-inst
        exit
    then
    h# 20 over= if                          \ MICRON
        spi> h# ba h# bc within if          \ BA,BB family
            ['] war-n25q-methods jedec-inst
            exit
        then
    then
    h# ef over= if                          \ W25M512
        spi> h# 71 = if
            ['] w25m-methods describes
            ['] seq-w25m inblob sequencer
            df-wait h# b7 df-wcmd df-wait   \ EN4B command
            d# 32 addressing
            enter-xip autotune
            exit
        then
    then
    unified-man
    ;

end-driver
