;***************************************************************
;* this is an arm-function
;***************************************************************

    .armfunc _c_int00
    .global  _c_int00

;***************************************************************
;* TI includes
;***************************************************************

    .asg    _args_main,   ARGS_MAIN_RTN
    .global ARGS_MAIN_RTN
    .global __TI_auto_init

    .global __stack
__stack:.usect  ".stack", 0, 4
    .global __STACK_SIZE

c_stack         .long    __stack
c_STACK_SIZE    .long    __STACK_SIZE

;***************************************************************
;* FUNCTION DEF: _c_int00
;***************************************************************
_c_int00: .asmfunc
	LDR     sp, c_stack
	LDR     r0, c_STACK_SIZE
	ADD     sp, sp, r0

    ; Perform all the required initilizations:
    ;  - Process BINIT Table
    ;  - Perform C auto initialization
    ;  - Call global constructors)
    BL    __TI_auto_init

    ; CALL APPLICATION
    BL    ARGS_MAIN_RTN

.end
