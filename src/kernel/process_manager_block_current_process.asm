	.global process_manager_block_current_process
	.global process_manager_block_current_process_c

process_manager_block_current_process_continue:
	; here the process will continue after change from BLOCKING to RUNNING

	; enable interrupts
	MRS R12, CPSR
	BIC R12, R12, #192
	MSR CPSR_c, R12

	LDMFD R13!, {R0, R12}

	MOV PC, R14

process_manager_block_current_process:
	STMFD R13!, {R0, R12}

	ADR R0, process_manager_block_current_process_continue		; load label into R0

	; Disable interrupts
	MRS R12, CPSR
	ORR R12, R12, #192
	MSR CPSR_c, R12

	SWI #0x1													; process_context_save

	B process_manager_block_current_process_c
