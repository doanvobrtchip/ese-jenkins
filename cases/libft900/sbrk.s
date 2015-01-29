        .section .text

        .global _sbrk
_sbrk:

        lda     $r1,__heap_ptr
        add     $r2,$r1,$r0

        sub     $r3,$sp,1000  # stack safety area, 1000 bytes
        cmp     $r2,$r3
        jmpc    lt,sbrkok
        # No room, so fail by returning -1
        ldk     $r0,-1
        return
sbrkok:
        sta     __heap_ptr,$r2
        move    $r0,$r1
        return

        .section .data
__heap_ptr:
        .long _end
