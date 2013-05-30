////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
//
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x000F0000

MEMORY
{
   SRAM:  	 			ORIGIN = 0x40200000  LENGTH = 0x0000FFC4

   DDR:				  	ORIGIN = 0x80000000  LENGTH = 0x00400000 // 4MB
   DDR_USER:			ORIGIN = 0x80400000  LENGTH = 0x03C00000 // 64 - 4MB

   INTERRUPT_VECTORS:	ORIGIN = 0x4020FFC8  LENGTH = 0x0000001C
}

stackSize = 0x20000;

SECTIONS
{
   .interruptvectors	> INTERRUPT_VECTORS {
   		*(.interruptvectors)
   	}

   .const      > DDR
   .bss        > DDR
   .far        > DDR

   .stack      > DDR
   .data       > DDR
   .cinit      > DDR
   .cio        > DDR

   .text       > SRAM
   .sysmem     > DDR
   .switch     > DDR

   .stackArea > DDR {
       . = align(4);
       . = . + stackSize;
       stackIRQ = .;
       . = . + stackSize;
       stackSupervisor = .;
       . = . + stackSize;
       stackAbort = .;
       . = . + (4 * stackSize);
       stackSystem = .;
   }
}
