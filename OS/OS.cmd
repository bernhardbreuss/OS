////////////////////////////////////////////////////////
// Linker command file for BeagleBoard)
//
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x000F0000

MEMORY
{
   SRAM:  	 			ORIGIN = 0x40200000  LENGTH = 0x0000FFC0

   DDR:				  	ORIGIN = 0x80000000  LENGTH = 0x00400000 // 4MB
   																 // 10 MB hole for process which are loaded with the OS binary
   LED1_USER:			ORIGIN = 0x80400000  LENGTH = 0x00100000 // 1MB (section size)
   DDR_USER:			ORIGIN = 0x80E00000  LENGTH = 0x03200000 // 64 - 10 - 4MB

   INTERRUPT_VECTORS:	ORIGIN = 0x4020FFC0  LENGTH = 0x00000020

   FOOBAR:			ORIGIN = 0x00E00000  LENGTH = 0x03200000 // 64 - 10 - 4MB
}

stackSize = 0x20000;

SECTIONS
{
   .interruptvectors	> INTERRUPT_VECTORS {
   		interruptvectors = .;
   		*(.interruptvectors)
   	}

	.foos load = LED1_USER, run = FOOBAR {
		led1_user.obj { RUN_START(led1_user_virtual), LOAD_START(led1_user_physical), SIZE(led1_user_size) }
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
