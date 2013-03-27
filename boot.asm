;***************************************************************
;* this is an arm-function
;***************************************************************

    .armfunc _c_int00
    .global  _c_int00

;***************************************************************
;* TI includes
;***************************************************************

    .global ARGS_MAIN_RTN
    .asg    __args_main,   ARGS_MAIN_RTN
    .import main
    .global __TI_auto_init


;***************************************************************
;* FUNCTION DEF: _c_int00
;***************************************************************

_c_int00: .asmfunc

    ; SET System-Mode
    CPS   0x1F

    ; Enable Interrupts
    MRS   R12, CPSR
    BIC   R12, R12, #192
    MSR   CPSR_cf, R12

    ; Perform all the required initilizations:
    ;  - Process BINIT Table
    ;  - Perform C auto initialization
    ;  - Call global constructors)
    BL    __TI_auto_init

	;CPS   0x10

    ; CALL APPLICATION
    ;BL    ARGS_MAIN_RTN
    BL    main

.end
