        .global usleep
        .global csleep
usleep:
        mul.l   $r0,$r0,90
        jmp     csleep

# Sleep for $r0 cycles
sleep:
        sub.l   $r0,$r0,3
csleep:
        cmp.l   $r0,0
        jmpc    gt,sleep
        return
