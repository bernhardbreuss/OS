	.global swi_handler
	.global ipc_handle_syscall

	.global process_context_save
	.global process_context_load

_handle_syscall:
	BL	ipc_handle_syscall		; call syscall handler
	LDMFD R13!, {R1-R2}			; load user PC and SPSR
	MSR CPSR_cxsf, R2			; return back to user mode
	MOV PC, R1					; back to user program

swi_handler:
	STMFD R13!, {R11-R12}		; backup R11, R12

	LDR R11, [R14, #-4]			; load SWI instruction
	BIC R11, R11, #0xFF000000	; mask off the MSB 8 bits

	CMP R11, #0x0				; check for syscall
	BEQ swi_handler_syscall

	CMP R11, #0x1				; check for context save
	BEQ swi_handler_context_save

	CMP R11, #0x2				; check for context load
	BEQ swi_handler_context_load

	; else invalid operation
	LDMFD R13!, {R11-R12}
	MOVS PC, R14

swi_handler_syscall:
	MRS R12, SPSR				; load SPSR into R12

	MOV R11, R14				; load R14 (user PC) into R11

	MSR CPSR_c, #0x1F			; change to system mode now
	STMFD R13!, {R11-R12}		; store user PC and SPSR

	MSR CPSR_c, #0x13			; change back to svc mode

	ORR R12, R12, #0x1F			; after interrupt handling, this
	MSR SPSR_c, R12				; process should be in system mode

	LDMFD R13!, {R11-R12}		; load backuped R11, R12

	ADRS PC, _handle_syscall	; end interrupt

swi_handler_context_save:
	LDMFD R13!, {R11-R12}
	STMFD R13!, {R0, R14}		; place user PC on top of stack
	BL process_context_save
	LDMFD R13!, {R0, R14}
	MOVS PC, R14

swi_handler_context_load:
	ADD R13, R13, #8			; release space of R11-R12
	B process_context_load
