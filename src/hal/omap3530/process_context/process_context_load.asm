	.global process_context_load

	.global process_context_pointer
_process_context_pointer: .field process_context_pointer, 32

	.global stackIRQ
_stackIRQ: .field stackIRQ, 32

process_context_load:
	LDR R14, _process_context_pointer	; load context pointer into R14
	LDR R14, [R14]						; load context address
	ADD R14, R14, #64					; move pointer to the end of the saved context

	LDR R12, [R14]						; load user CPSR into R12
	MSR SPSR_cxsf, R12					; set SPSR to user CPSR from context

	LDMDB R14, {R0-R14}^				; load stored user registers
	LDR R14, [R14, #-64]				; load user pc into R14

	MOVS PC, R14						; return to user process
