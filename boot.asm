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


	.global stackIRQ
	.global stackSupervisor
	.global stackAbort
	.global stackSystem

_stackIRQ			.long	stackIRQ
_stackSupervisor	.long	stackSupervisor
_stackAbort			.long	stackAbort
_stackSystem		.long	stackSystem

;***************************************************************
;* FUNCTION DEF: _c_int00
;***************************************************************
	.global call_this_method_form_boot_asm

_c_int00: .asmfunc

	CPS	0x12 ; IRQ
	LDR	sp, _stackIRQ

	CPS 0x13 ; SWI / Supervisor
	LDR sp, _stackSupervisor

	CPS 0x17 ; Abort
	LDR sp, _stackAbort

    ; SET System-Mode
    CPS	0x1F
    LDR	sp, _stackSystem


    ; Enable Interrupts
    MRS   R12, CPSR
    BIC   R12, R12, #192
    MSR   CPSR_cf, R12

    ; Perform all the required initilizations:
    ;  - Process BINIT Table
    ;  - Perform C auto initialization
    ;  - Call global constructors)
    BL    __TI_auto_init

    ; CALL APPLICATION
    BL    ARGS_MAIN_RTN

.end
