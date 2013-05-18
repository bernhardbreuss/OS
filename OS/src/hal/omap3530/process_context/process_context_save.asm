	.global process_context_save

	.global process_context_pointer
_process_context_pointer: .field process_context_pointer, 32


process_context_save:
	STMFD R13!, {R12, R14}				; backup R12 and R14 on stack
	LDR R14, _process_context_pointer	; load context pointer into R14
	LDR R14, [R14]						; load context address into R14

	LDR R12, [R13, #8]					; load user PC
	STMIA R14!, {R12}					; store user PC to context
	LDR R12, [R13]						; load backed up R12
	STMIA R14, {R0-R14}^				; store user registers to context
	MRS R12, SPSR						; load user CPSR into R12
	STR R12, [R14, #60]					; store user CPSR to context

	LDMFD R13!, {R12, R14}
	MOV PC, R14
