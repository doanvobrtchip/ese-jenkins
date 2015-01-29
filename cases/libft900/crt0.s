      .equ MAGIC       , 0x19470
      .equ MICROSECONDS, 0x1fff4
      .equ DEBUG       , 0x1fff8
      .equ EXITEXIT    , 0x1fffc
      .equ UART        , 0x10004

      .equ SCRATCH     , 0x1fc00
      .equ PMBASE      , 0x1fc80

.global _start
_start:

        jmp     0x3fffc
        # jmp     codestart
        jmp     ft900_watchdog
        jmp     interrupt_0
        jmp     interrupt_1
        jmp     interrupt_2
        jmp     interrupt_3
        jmp     interrupt_4
        jmp     interrupt_5
        jmp     interrupt_6
        jmp     interrupt_7
        jmp     interrupt_8
        jmp     interrupt_9
        jmp     interrupt_10
        jmp     interrupt_11
        jmp     interrupt_12
        jmp     interrupt_13
        jmp     interrupt_14
        jmp     interrupt_15
        jmp     interrupt_16
        jmp     interrupt_17
        jmp     interrupt_18
        jmp     interrupt_19
        jmp     interrupt_20
        jmp     interrupt_21
        jmp     interrupt_22
        jmp     interrupt_23
        jmp     interrupt_24
        jmp     interrupt_25
        jmp     interrupt_26
        jmp     interrupt_27
        jmp     interrupt_28
        jmp     interrupt_29
        jmp     interrupt_30
        jmp     interrupt_31
        jmp     0x3fff8

codestart:
        jmp     init
.global _exithook
_exithook:               # Debugger uses '_exithook' at 0x90 to catch program exit
        return
init:
        ldk     $r0,0x80
        sta.b   0x100e3,$r0

        # Initialize DATA
        ldk.l   $r0,__data_load_start
        ldk.l   $r1,__data_load_end
        ldk.l   $r2,0

        jmp     .dscopy
.dsloop:
        # Copy PM[$r0] to RAM $r2
        lpmi.l  $r3,$r0,0
        add.l   $r0,$r0,4
        sti.l   $r2,0,$r3
        add.l   $r2,$r2,4
.dscopy:
        cmp.l   $r0,$r1
        jmpc    lt,.dsloop

        # Zero BSS
        ldk.l   $r0,_bss_start
        ldk.l   $r2,_end
        sub.l   $r2,$r2,$r0
        ldk.l   $r1,0
        memset.l  $r0,$r1,$r2

        sub.l   $sp,$sp,24  # Space for the caller argument frame
        call    ft900_init
        # ldk     $r0,'o'
        # call    emit
        # ldk     $r0,'k'
        # call    emit
        # call    cr
        call    main

.global _exit
_exit:
        sta.l     EXITEXIT,$r0    # simulator end of test
        call      ft900_exit
forever:
        jmp forever

.global streamout
streamout:
        ldk.l   $r2,UART
        streamout.b $r2,$r0,$r1
        return

        # Macro to construct the interrupt stub code.
        # it just saves r0, loads r0 with the int vector
        # and branches to interrupt_common.

        .macro  inth i=0
interrupt_\i:
        push    $r0     # {
        lda     $r0,(vector_table + 4 * \i)
        jmp     interrupt_common
        .endm

        inth    0
        inth    1
        inth    2
        inth    3
        inth    4
        inth    5
        inth    6
        inth    7
        inth    8
        inth    9
        inth    10
        inth    11
        inth    12
        inth    13
        inth    14
        inth    15
        inth    16
        inth    17
        inth    18
        inth    19
        inth    20
        inth    21
        inth    22
        inth    23
        inth    24
        inth    25
        inth    26
        inth    27
        inth    28
        inth    29
        inth    30
        inth    31
        inth    32

        # On entry: r0, already saved, holds the handler function
interrupt_common:
        push    $r1     # {
        push    $r2     # {
        push    $r3     # {
        push    $r4     # {
        push    $r5     # {
        push    $r6     # {
        push    $r7     # {
        push    $r8     # {
        push    $r9     # {
        push    $r10    # {
        push    $r11    # {
        push    $r12    # {
        push    $cc     # {

        calli   $r0

        pop     $cc     # }
        pop     $r12    # }
        pop     $r11    # }
        pop     $r10    # }
        pop     $r9     # }
        pop     $r8     # }
        pop     $r7     # }
        pop     $r6     # }
        pop     $r5     # }
        pop     $r4     # }
        pop     $r3     # }
        pop     $r2     # }
        pop     $r1     # }
        pop     $r0     # } matching push in interrupt_0-31 above
        reti

        # Null function for unassigned interrupt to point at
nullvector:
        return

        .section .data
        .global vector_table
vector_table:
        .rept 33
                .long   nullvector
        .endr

        .section .text
.global __gxx_personality_sj0
__gxx_personality_sj0:

.global _times
_times:
        lda.l     $r1,MICROSECONDS
        sti.l   $r0,0,$r1
        ldk.l   $r1,0
        sti.l   $r0,4,$r1
        sti.l   $r0,8,$r1
        sti.l   $r0,12,$r1
        return

        .if 0   # {

        .global donothing
donothing:      return

        .global blinkit
blinkit:
        # ldk     $r0,0x80
        # sta.b   0x100e3,$r0

        lpm.l   $r0,k_55
        sta.l   0x10060,$r0
loop1:
        add.l   $r0,$r0,1
        bexts.l $r1,$r0,(1<<5)|21
        sta.l   0x10084,$r1
        jmp     loop1
k_55:
        .long   0x55555555

        .global hex8
hex8:
        push    $r5
        move    $r5,$r0
        lshr    $r0,$r0,16
        call    hex4
        move    $r0,$r5
        call    hex4
        pop     $r5
        return

hex4:
        push    $r5
        move    $r5,$r0
        lshr    $r0,$r0,8
        call    hex2
        move    $r0,$r5
        call    hex2
        pop     $r5
        return

hex2:
        push    $r5
        move    $r5,$r0
        lshr    $r0,$r0,4
        call    hex1
        move    $r0,$r5
        call    hex1
        pop     $r5
        return

hex1:
        and     $r0,$r0,15
        cmp     $r0,10
        jmpc    lt,digit
        add     $r0,$r0,7
digit:
        add     $r0,$r0,0x30
        jmp     emit

cr:
        ldk     $r0,13
        call    rawemit
        ldk     $r0,10
        jmp     rawemit

space:
        ldk     $r0,32
        jmp     emit

emit:
        cmp     $r0,10
        jmpc    z,cr
rawemit:
        push    $r1
.waitready:
        lda.b   $r1,0x10325
        and     $r1,$r1,0x20
        cmp.b   $r1,0
        jmpc    z,.waitready
        sta.b   0x10320,$r0
        pop     $r1
        return 

        .endif
