;***************************************************************
;* this is an arm-function
;***************************************************************

    .armfunc _c_int00
    .global  _c_int00

;***************************************************************
;* TI includes
;***************************************************************

    .global main
    .global __TI_auto_init

    .global __stack
__stack:.usect  ".stack", 0, 4
    .global __STACK_SIZE

c_stack         .long    __stack
c_STACK_SIZE    .long    __STACK_SIZE

	.global std_adapter_adapt
	.global ARGS_MAIN_RTN
	.asg	main,   ARGS_MAIN_RTN
	.global argument_helper_parse
	.global ARGV_ADDR
_argv_addr		.long	ARGV_ADDR
	.global _os_std_exit_process

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

    BL std_adapter_adapt

	BL argument_helper_parse
	LDR R1, _argv_addr

    ; CALL APPLICATION
    BL    main
    BL _os_std_exit_process

.end
