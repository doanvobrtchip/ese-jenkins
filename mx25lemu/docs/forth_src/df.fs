\ SPI MASTER -------------------------------------------------

\ Uses REG_SPIM
\   bit 0       MOSI
\   bit 4       CS
\   bit 5       SCK
\ and REG_SPIM
\   bit 1       MISO
\

: idle      h# 10 vc.REG_SPIM ! ;

: out1 d# 1 and ;fallthru
: outN ( x -- )
    h# 20 over+ over   ( v v|CLK v )
    vc.REG_SPIM a! a! noop ! ;
: out4 h# f and outN ;

: >spi ( x -- )
    d# 7 2duprshiftnip out1
    d# 6 2duprshiftnip out1
    d# 5 2duprshiftnip out1
    d# 4 2duprshiftnip out1
    d# 3 2duprshiftnip out1
    d# 2 2duprshiftnip out1
    d# 1 2duprshiftnip out1
                       out1 ;

: in1 ( u -- u' )
    2*
    vc.REG_SPIM @ d# 2 and +
    h# 01 outN ;

: in8 ( -- x )  \ x is 2* the SPI input byte
    d# 0 in1 in1 in1 in1 in1 in1 in1 in1 ;

: spi>  in8 2/ ;

: lspi> ( -- x32 )
    in8 d# 1 rshift
    in8 d# 7 lshift +
    in8 d# 15 lshift +
    in8 d# 23 lshift + ;

: 32bit ( -- f ) flash-addr w@ d# 32 = ;

: addr>spi   ( x -- ) \ Big endian address to SPI, either 24 or 32 bit
    dup d# 24 rshift
    32bit if
        >spi
    else
        abort" illegal flash address"
    then
    dup d# 16 rshift >spi
    dup d# 8 rshift >spi
    >spi ;

: l>spi  ( x32 -- ) \ Little endian 32-bit to SPI
    dup >spi
    dup d# 8 rshift >spi
    dup d# 16 rshift >spi
        d# 24 rshift >spi ;

: in4
    h# 20 vc.REG_SPIM @!
;

: qpi>  ( -- x ) in4 d# 4 lshift in4 + ;

: spi
    h# 11 ;fallthru
: dir
    vc.REG_SPIM_DIR ! ;

: qout
    h# 1f vc.REG_SPIM !
    h# 1f dir ;

: qin
    h# 10 dir ;

: >qpi ( u -- ) dup d# 4 rshift out4 out4 ;

\ DATAFLASH COMMON -------------------------------------------

: describes
    flash-methods w! ;

: dispatch
    flash-methods w@ execute ;

: df-wait   d# 0 dispatch ;

: func:flasherase                           d# 1 dispatch ;
: func:flashwrite
    cmd.get d# 256 aligns
    cmd.get d# 256 aligns
    ?dup 0= if drop exit then
    d# 2 dispatch ;
: func:flashread
    cmd.get 4aligns
    cmd.get d# 64 aligns
    swap
    cmd.get 4aligns
    ;fallthru
: df-read ( src dst u )
    ?dup 0= if 2drop exit then
    d# 3 dispatch ;
: leave-xip                                 d# 4 dispatch ;
: enter-xip                                 d# 5 dispatch ;

: df-ff 
    h# ff ;fallthru
: df-cmd ( u -- )
    idle >spi ;

: df-wcmd
    h# 06 df-cmd df-cmd ;

: df-status ( -- status )
    h# 05 df-cmd spi> ;

: df-setstatus ( u -- )
    h# 01 df-wcmd >spi ;

: df-setstatus16 ( u0 u1 )
    dup df-setstatus 256/ >spi ;

: jedec-wait   \ wait until WIP is zero
    begin
        df-status
        h# 1 and 0=
    until ;

: single ( a -- ) \ read a single 64-byte page into vc.REG_ESPIM_WINDOW
    vc.REG_ESPIM_ADD !
    d# 1 vc.REG_ESPIM_COUNT !
    d# 1 vc.REG_ESPIM_TRIG a!
    begin dup@ until drop ;

: espim-src ( src dst u -- dst u )
    rot vc.REG_ESPIM_ADD ! ;

: (espim-rd) ( src dst u -- )
    espim-src
    idle
    dup d# 6 rshift vc.REG_ESPIM_COUNT !   ( u dst )
    d# 1 vc.REG_ESPIM_TRIG !
    bounds begin
        vc.REG_ESPIM_TRIG begin dup@ until drop

        >r vc.REG_ESPIM_WINDOW
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ nip
        2dup=
    until 2drop ;

: espim-rd1 ( src dst u -- )     \ Read 1 window at a time
    espim-src
    d# 1 vc.REG_ESPIM_COUNT !
    bounds begin
        d# 1 vc.REG_ESPIM_TRIG !
        >r vc.REG_ESPIM_WINDOW
        vc.REG_ESPIM_TRIG begin dup@ until drop

        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ >r
        dup@+ r> a!+ nip

        d# 64 vc.REG_ESPIM_ADD +!
        2dup=
    until 2drop ;

\ espim-rd: Used in XIP mode. Either a continuous or a safer slow read.

: espim-rd ( src dst u )
    dup>r d# -64 and
    dup if
        3dup
        vc.REG_PCLK @ if espim-rd1 else (espim-rd) then
    then
    tuck + >r +         ( src' R: dst' )
    single
    vc.REG_ESPIM_WINDOW r> r> d# 63 and
    move ;

\ Winbond and Macronix need an initial read to get into XIP mode
\ send: cmd, 24-bit address, pp, then reads and discards

: firstread ( pp cmd -- )
    df-cmd
    h# 1f dir

    32bit if
        h# 00 >qpi
    then
    h# 00 >qpi
    h# 00 >qpi
    h# 00 >qpi

    >qpi

    h# 10 dir
    spi> drop
    spi idle ;
: df-qcmd
    idle qout >qpi ;
: df-qwcmd
    h# 06 df-qcmd df-qcmd ;

: qfirstread ( pp cmd -- )
    df-qcmd
    32bit if
        h# 00 >qpi
    then
    h# 00 >qpi
    h# 00 >qpi
    h# 00 >qpi
          >qpi ;fallthru
: 10dummy                               \ 10 dummy clocks
    qin
    d# 5 begin
        qpi> drop eol
    until drop
    idle ;

: sequencer ( a -- ) \ configure the sequencer 
    vc.REG_ESPIM_READSTART ;fallthru
: 20move
    d# 20 HImove ;

: inblob ( a -- a' )    \ Turn a ticked address into a full address
    h# 7800 -
    vc.RAM_ROMSUB + ;

: sha1key
    vc.REG_SHA1KEY @ ;

: addressing ( u -- ) \ set addressing size
    flash-addr w! ;

\ COMMON JEDEC OPERATIONS ------------------------------------

: jedec-reset
    h# 66 df-cmd h# 99 df-cmd
    idle d# 50 us ;

: jedec-manufacturer
    h# 9f df-cmd spi> ;

: madeby ( u -- f f )    \ Was this part made by manufacturer u?
    jedec-manufacturer = dup ;

: jedec-erase ( -- )
    leave-xip
    h# 01 df-wcmd h# 00 >spi df-wait    \ Try to clear BP3-0
    h# c7 df-wcmd                       \ Full-chip erase
    df-wait
    enter-xip ;

: jedec-write ( dst u -- )
    leave-xip
    bounds begin
        2dupxor
    while
        h# ff overand 0= if
            df-wait
            h# 02 df-wcmd dup addr>spi
        then
        cmd.get cmd.sync l>spi cell+
    repeat 2drop
    df-wait
    enter-xip ;

: jedec-startrd ( src )
    h# 03 df-cmd addr>spi ;

: jedec-rd ( src dst u )
    rot jedec-startrd
    bounds begin
        lspi> swap a!+
        2dup=
    until 2drop ;

: large (  -- f ) \ True if the device has >24 bit addresses
    vc.REG_FLASH_SIZE @ d# 16 > ;

\ DIRECT ACCESS API ------------------------------------------

: func:flashdetach
    leave-xip idle
    h# 00 dir
    vc.FLASH_STATUS_DETACHED ;fallthru
: setstatus
    vc.REG_FLASH_STATUS ! ;

: func:flashdesel
    spi idle ;

: func:flashspitx
    cmd.get
    begin
        d# -4 overand
    while
        cmd.get l>spi
        d# 4 -
    repeat
    ?dup if
        cmd.get
        begin
            dup >spi
            d# 8 rshift
            swap 1- tuck 0=
        until 2drop
    then ;

: func:flashspirx
    cmd.get cmd.get ( a n )
    bounds begin
        2dupxor
    while
        spi> over c!
        1+
    repeat 2drop ;

\ WORKAROUNDS ------------------------------------------------

: n25q-wait
    begin
        h# 70 df-cmd spi> h# 80 and
    until ;

: n25q-erase ( -- )
    leave-xip
    vc.REG_FLASH_SIZE @ d# 64 = if          \ N25Q512 cannot do a full erase
        h# c4 df-wcmd d# 0 addr>spi         \ Die erase
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
    jmp jedec-rd
    jmp (noop)
    jmp (noop)

: workarounds ( id16 -- id16 )
    h# 0001 overor h# 20bb = if
        ['] war-n25q-methods describes
        h# 01 df-wcmd h# 00 >spi df-wait    \ Try to clear BP3-0
    then ;

: df-workarounds
    jedec-manufacturer 256* spi> +
    (workarounds) w@ execute
    drop ;

\ DISPATCH ---------------------------------------------------

: default-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp jedec-rd
    jmp (noop)
    jmp (noop)

\ ATTACH AND FAST --------------------------------------------

\ For densities 2 gigabits or less, bit-31 is set to 0b. The
\ field 30:0 defines the size in bits.
\ 
\ For densities 4 gigabits and above, bit-31 is set to
\ 1b. The field 30:0 defines ‘N’ where the density is
\ computed as 2^N bits (N must be >= 32).

: sfdp-density ( density -- )   \ Sets REG_FLASH_SIZE (in MB)
    dup0< if
        h# ff and d# 32 - d# 512 swap lshift
    else
        d# 512         ( density kbsize )
        begin
            over 0>
        while
            2/ swap 2* swap
        repeat
        nip
    then ;

: qsfdp ( lo mid hi )   \ start an SFDP query
    h# 5a df-cmd
    >spi
    >spi
    >spi
    spi> drop ;         \ dummy cycle

: find4k.16 ( u16 )
    h# ff overand d# 12 =
    if
        d# 8 rshift h# ff and df-erase4k w!
    else
        drop
    then ;

: find4k ( u32 )
    dup find4k.16 d# 16 rshift find4k.16 ;

: sfdp ( -- size )
    h# 00 h# 00 h# 00  qsfdp        \ Read "SFDP" signature
    lspi> h# 50444653 = if
        h# 0c h# 00 h# 00  qsfdp    \ Read from 00000C
        spi> spi> spi>  ( lo mid hi )

        qsfdp
        lspi> drop                  \ 1
        lspi> sfdp-density          \ 2
        lspi> drop                  \ 3
        lspi> drop                  \ 4
        lspi> drop                  \ 5
        lspi> drop                  \ 6
        lspi> drop                  \ 7
        lspi> find4k                \ 8
        lspi> find4k                \ 9
    else
        d# 0
    then ;

\ Micron parts need a special sequences to recover
\ See "Power Loss Recovery Sequence" in their spreadsheet.
\ Of course, any time VC3 is reset the flash is in an unknown
\ state, so this is needed for every reset.

: pulses ( n -- )
    d# 0 begin
        h# 9 out4
        1+ 2dup=
    until 2drop
    idle ;

: micron-recovery
    d# 7 pulses
    d# 9 pulses
    d# 13 pulses
    d# 17 pulses
    d# 25 pulses
    d# 33 pulses ;

: func:flashattach
    spi idle
    ['] default-methods describes
    d# 24 addressing
    micron-recovery
    df-ff           \ Exit XIP, exit QPI
    jedec-reset
    sfdp
    dup vc.REG_FLASH_SIZE !
    dup if
        df-workarounds
    else
        vc.REG_SPIM off
        h# 1d dir                   \ Drive SSN, SCLK, MOSI, IO2, IO3 low
    then
    if vc.FLASH_STATUS_BASIC else vc.FLASH_STATUS_DETACHED then
    setstatus ;

: confirm ( a u -- f )
    sha1key if
        hmac
        d# 20 bounds
        true >r
        begin
            2dupxor
        while
            dup@ lspi> = r> and >r
            cell+
        repeat 2drop
        r>
    else
        2drop true
    then ;

: read-bad ( -- f ) \ is a read bad?
    d# 0 single
    vc.REG_ESPIM_WINDOW @
    h# 92fbdf70 <> ;

: check-basic ( 0 -- 0|E001 ) \ Is flash attached, in BASIC mode
    vc.REG_FLASH_STATUS @ vc.FLASH_STATUS_BASIC <> h# 01 ;fallthru
: errorcode ( 0 fail code -- 0|code )
    h# e000 or and or ;

: check-header ( 0 -- 0|E002 ) \ Sec0 starts with the header?
    d# 0 jedec-startrd
    lspi> vc.FLASH_HEADER <>
    h# 02 errorcode ;

: check-signature ( 0 -- 0|E003 ) \ Sec0 payload signature valid?
    loadarea lspi> d# 1 d# 1023 clamp
    2dup bounds 
    begin
        2dupxor
    while
        spi> over c! 1+
    repeat 2drop
    confirm 0=
    h# 03 errorcode ;

: check-install ( 0 -- 0|E004 ) \ Does the installer return TRUE?
    \ Copy from the staging area 
    \ - first 20 bytes to REG_ESPIM_READSTART
    \ - remainder to ROMSUB area
    loadarea d# 6 +
    vc.REG_ESPIM_READSTART d# 20 append drop    ( src' )
    loadarea dup@+ swap w@   ( src' dst u )
    d# 20 /string
    HImove
    d# 3 vc.REG_ROMSUB_SEL !.
    h# 7ffe execute 0=
    h# 04 errorcode ;

: check-actual ( 0 -- 0|E005 ) \ Do two reads yield expected value?
    read-bad read-bad or
    h# 05 errorcode ;

: func:flashfast
    d# 0      check-basic
    dup 0= if
                  check-header
        dup 0= if check-signature then
        dup 0= if check-install then
        dup 0= if check-actual then
        dup 0= if
            vc.FLASH_STATUS_FULL setstatus
        else
            ['] default-methods describes
        then
    then
    cmd.put ;

\ FLASH UPDATE -----------------------------------------------

\ transition:
\ bit 31: there was at least one 0->1 bit transition. Erase.
\ bit  0: there was at least one 1->0 bit transition. Write.
\ So return value is one of 0, -1, 1

: transition ( new old -- code )
    2dup invert and 0<>          >r
    swap invert and 0<> d# 1 and r> or ;

: jedec<> ( a fa u -- f )   \ bit 31: erase, 0: write
    swap jedec-startrd
    false >r
    bounds begin
        dup@+ lspi> ( mem flash )
        transition r> or dup>r
        0< if drop dup then
        2dup=
    until 2drop r> ;

: jedec-copy ( fa a u -- )
    swap >r
    bounds begin
        2dupxor
    while
        h# ff overand 0= if
            df-wait
            h# 02 df-wcmd dup addr>spi
        then
        r> dup@+ l>spi >r
        cell+
    repeat 2drop rdrop
    df-wait ;

4096 constant 4K

: func:flashupdate
    cmd.get 4K aligns
    cmd.get 4aligns >r
    cmd.get 4K aligns     ( dst u  R: src )

    leave-xip
    bounds begin
        2dupxor
    while
        r@ over 4K jedec<>
        dup 0< if
            df-erase4k w@ df-wcmd over addr>spi
        then
        d# 1 and if
            dup r@ 4K jedec-copy
        then
        r> 4K + >r 4K +
    repeat 2drop rdrop
    enter-xip ;

\ UNIFIED DRIVER ---------------------------------------------

: autotune
    h# 0fc0 single
    [ vc.REG_ESPIM_WINDOW 16 + ] literal @ h# 5a5a5a5a xor 1+ d# 2 u< if
        [ vc.REG_ESPIM_WINDOW 32 + ] literal @
        d# 28 lshift d# 28 rshift
        dup df-tuned !
        vc.REG_ESPIM_READSTART +!
    then ;

create generic-seq-32
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

vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq

            ]fin

create seq-lastread
            beg[
vc.SS_A5    seq       \ ADDR 6
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QF    seq       \ FF means finish
vc.SS_QF    seq
vc.SS_QI    seq       \ 4 turnaround cycles
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

create seq-lastread-32
            beg[
vc.SS_A7    seq       \ ADDR 8
vc.SS_A6    seq
vc.SS_A5    seq
vc.SS_A4    seq
vc.SS_A3    seq
vc.SS_A2    seq
vc.SS_A1    seq
vc.SS_A0    seq
vc.SS_QF    seq       \ FF means finish
vc.SS_QF    seq
vc.SS_QI    seq       \ 4 turnaround cycles
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
vc.SS_QI    seq
            ]fin

ramcreate   seqsave 20 allot

: df-lastread                   \ read request for 00000000 with continue bits FF
    seqsave
    vc.REG_ESPIM_READSTART over 20move
    32bit if
        seq-lastread-32
    else
        seq-lastread
    then
    sequencer
    dup@ vc.REG_ESPIM_READSTART !   \ restore calibrated RESTART
    d# 0 single
    sequencer ;

: e3-xip    h# A5 h# E3 firstread ;
: e3-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp e3-xip

: eb-xip    h# A5 h# EB firstread ;
: eb-methods
    2*route ;fallthru
    jmp jedec-wait
    jmp jedec-erase
    jmp jedec-write
    jmp espim-rd
    jmp df-lastread
    jmp eb-xip

: jedec-inst-status ( methods s16 -- )
    df-setstatus16 ;fallthru
: jedec-inst ( methods -- )
    df-wait
    describes
    large if
        h# b7 df-wcmd                       \ EN4B command
        d# 32 addressing
        generic-seq-32 sequencer
    then
    df-wait enter-xip autotune ;

\ To activate XIP requires two steps. First, enable XIP by
\ setting volatile configuration register bit 3 to 0. Next,
\ drive the XIP confirmation bit to 0 during the next FAST
\ READ operation. XIP is then active. (p.69)
\ QUAD INPUT/OUTPUT FAST READ Command – STR
\ (Extended). p.48
: rom-n25q-xip
    h# 61 df-wcmd h# 6f >spi idle           \ enter quad SPI, disable HOLD
    h# 81 df-qwcmd h# a3 >qpi               \ WRITE VOLATILE, 10 dummy, XIP
    h# 00 h# eb qfirstread ;

: rom-n25q-spi
    h# ff df-qcmd                           \ leave XIP
    h# ff >qpi
    h# ff >qpi
    h# ff >qpi
    h# ff >qpi 10dummy
    h# 61 df-qwcmd h# ff >qpi               \ leave quad SPI
    spi ;

: dieerase ( addr -- )
    h# c4 df-wcmd addr>spi ;

: n25q-erase-fast ( -- )
    leave-xip
    vc.REG_FLASH_SIZE @ d# 64 = if          \ N25Q512: do DIE ERASE in 2 parts
        h# 0000000 dieerase                 \ Die erase
        n25q-wait
        h# 1000000 dieerase                 \ Die erase
    else
        h# c7 df-wcmd                       \ Bulk erase
    then
    n25q-wait
    enter-xip ;

: rom-n25q-methods
    2*route ;fallthru
    jmp n25q-wait
    jmp n25q-erase
    jmp jedec-write
    jmp espim-rd
    jmp rom-n25q-spi
    jmp rom-n25q-xip

: unified
    jedec-manufacturer ;fallthru
: unified-man ( manid -- t/f )
    h# 20 over= if                          \ N25Q
        ['] rom-n25q-methods jedec-inst exit
    then
    h# ef over= if                          \ W25Q
        ['] e3-methods h# 200 jedec-inst-status exit
    then
    h# c2 over= if                          \ MX25
        h# 40 df-setstatus
        ['] eb-methods jedec-inst exit
    then
    h# 01 over= if                          \ S25FL1
        ['] eb-methods h# 200 jedec-inst-status exit
    then
    drop false ;
