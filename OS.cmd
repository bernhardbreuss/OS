////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
//
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x00002000

MEMORY
{
   SRAM:  	 ORIGIN = 0x40200000  LENGTH = 0x00010000
   DDR:  	 ORIGIN = 0x80000000  LENGTH = 0x10000000
}

SECTIONS
{
   .const      > DDR
   .bss        > DDR
   .far        > DDR

   .stack      > DDR
   .data       > DDR
   .cinit      > DDR
   .cio        > DDR

   .text       > DDR
   .sysmem     > DDR
   .switch     > DDR
}
