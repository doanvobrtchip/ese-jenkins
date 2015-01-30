#include <stdio.h>
 
int main()
{
  puts("Hello world");
  printf("getchar()\n");
  int c = getchar();
  printf("got '%c'\n", c);
  printf("getchar()\n");
  c = getchar();
  printf("got '%c'\n", c);
  
  // At this point reading io 0x10325 in loop (lsr/icr on the UART)
  // 00340136 call 4d8 <__srget_r>
  // 003424f9 call 93e4 <__srefill_r>
  // 00341c86 call 7218 <__smakebuf_r>
  // 00343e43 call f90c <_fstat_r>
  // 003447f5 call 11fd4 <_fstat>
  // 00341cf3 call 73cc <_malloc_r>
  // 00341f75 call 7dd4 <__malloc_lock>
  // 00341f78 call 7de0 <__malloc_unlock>
  // 08340040 calli $r4 // CALL from (94d0) to (98cc) <__sread>
  // 00343f39 call fce4 <_read_r>
  // 003447e3 call 11f8c <_read>
  // 003447d1 call 11f44 <uart_rx> // Handled in uart_rx
  
  /* 

   00011f44 <uart_rx>:
   11f44:	25 03 01 c0 	c0010325 lda.b $r0,10325 <UART+0x321>
   11f48:	0c 42 e0 5d 	5de0420c btst.l $r0,20 <_start+0x20>
   11f4c:	d1 47 28 00 	002847d1 jmpc z,11f44 <uart_rx>
   11f50:	20 03 01 c0 	c0010320 lda.b $r0,10320 <UART+0x31c>
   11f54:	00 00 00 a0 	a0000000 return 

   */
   
   /*

	uint8_t uart_rx()
	{
	  while ((*uart1_lsr & UART1_LSR_DR) == 0)
		;
	  return *uart1_rhr;
	}

   */
  
  printf("Set a value: \n");
  int x;
  scanf("%i", &x);
  printf("Ok\n");
  printf("Value is: %i\n", x);
  printf("Bye!\n");  
  return 0;
}
