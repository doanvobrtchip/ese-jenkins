      .equ DEBUG       , 0x1fff8

.global _gettimeofday
_gettimeofday:
  return


.global _sbrk
_sbrk:
    lda.l   $r1,__heap_ptr
    add.l   $r2,$r1,$r0

    sub.l   $r3,$sp,1000  # stack safety
    cmp.l   $r2,$r3
    jmpc    lt,sbrkok
    # No room, fail by returning -1
    ldk.l   $r0,-1
    return

sbrkok:
    sta.l   __heap_ptr,$r2
    move.l  $r0,$r1
    return
  .section .data
__heap_ptr:
  .long _end
  .section .text

.global _read
_read:
  sta.l DEBUG,$r0
  sta.l DEBUG,$r1
  sta.l DEBUG,$r2

  return

.global _link
_link:
  sta.l DEBUG,$r0
  return

.global _kill
_kill:
  return
.global _getpid
_getpid:
  return

.global millisleep
millisleep:
        LDK.l   $r1,1000
        MUL.l   $r0,$r0,$r1
.global microsleep
microsleep:
        mul.l   $r0,$r0,33
        jmp     sleep_t
# Sleep for 3*R0 cycles
sleep:
        sub.l   $r0,$r0,1
sleep_t:
        cmp.l   $r0,0
        jmpc    nz,sleep
        return

.global __gxx_personality_sj0
__gxx_personality_sj0:

