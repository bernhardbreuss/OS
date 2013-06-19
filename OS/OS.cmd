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

   DDR:				  	ORIGIN = 0x80000000  LENGTH = 0x00400000 // 4MB (Kernel)
   USER_PROCESSES:		ORIGIN = 0x80400000  LENGTH = 0x00A00000 // 10MB user processes which are loaded with the OS binary
   DDR_USER:			ORIGIN = 0x80E00000  LENGTH = 0x03200000 // 50MB RAM for user processes (physical)

   VIRTUAL_PROCESSES:	ORIGIN = 0x00E00000  LENGTH = 0x03200000 // 50MB RAM for user processes (virtual)
   ARGUMENTS:			ORIGIN = 0x3FFFF000  LENGTH = 0x00001000
}

stackSize = 0x20000;
pageSize = 0x1000;

SECTIONS
{
   	.processes load = USER_PROCESSES, run = VIRTUAL_PROCESSES {
   		. = align(pageSize);
   		idle_process.obj { RUN_START(idle_process_virtual), LOAD_START(idle_process_physical), SIZE(idle_process_size) }
   	}

   .interruptvectors load = SRAM ALIGN = 4 {
   	    interruptvectors = .;
   	    *(.interruptvectors)
   }

   .const      > DDR
   .bss        > DDR
   .far        > DDR

   .stack      > DDR
   .data       > DDR
   .cinit      > DDR
   .cio        > DDR

   .text       >> SRAM | DDR
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

   .arguments  > ARGUMENTS {
		ARGS_ADDR = .;
		. += 500; // arguments max length
   		ARGV_ADDR = .;
   }

}
