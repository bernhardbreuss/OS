////////////////////////////////////////////////////////
// Linker command file for OS
//
// Basic configuration using only external memory
//

-stack           0x00002000
-heap            0x000F0000

MEMORY
{
   USER_SPACE:  	 	ORIGIN = 0x00000000  LENGTH = 0x3FFFF000
   ARGUMENTS:			ORIGIN = 0x3FFFF000  LENGTH = 0x00001000
}

stackSize = 0x20000;

ARGS_ADDR = 0x3FFFF000;
ARGV_ADDR = 0x3FFFF000 + 500;

SECTIONS
{
   .const      > USER_SPACE
   .bss        > USER_SPACE
   .far        > USER_SPACE
   .stack      > USER_SPACE
   .data       > USER_SPACE
   .cinit      > USER_SPACE
   .cio        > USER_SPACE
   .text       > USER_SPACE
   .sysmem     > USER_SPACE
   .switch     > USER_SPACE
}
