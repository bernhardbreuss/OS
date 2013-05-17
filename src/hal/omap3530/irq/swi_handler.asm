	.global swi_handler
	.global ipc_handle_syscall

	.global process_context_save
	.global process_context_load

	.global process_manager_change_process

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

	CMP R11, #0x1				; check for change process
	BEQ swi_handler_change_process

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

swi_handler_change_process:
	LDMFD R13!, {R11-R12}

	SUB R13, R13, #4			; place user PC on top of stack
	STR R14, [R13]

	BL process_context_save		; save context

	ADD R13, R13, #4			; pop user PC from stack

	BL process_manager_change_process

	B process_context_load		; load new process
