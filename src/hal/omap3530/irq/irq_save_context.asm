	.global irq_save_context

	.global irq_saved_context_pointer
_irq_saved_context_pointer: .field irq_saved_context_pointer, 32


irq_save_context:
	STMFD R13!, {R12, R14}	;backup R12 and R14 on stack
	LDR R14, _irq_saved_context_pointer
	LDR R14, [R14]

	LDR R12, [R13, #8]
	STMIA R14!, {R12}
	LDR R12, [R13]
	STMIA R14, {R13}^
	STMIB R14, {R0-R12,R14}^
	ADD R14, R14, #60 ; 60 = 15*4 because writeback isn't possible

	LDR R12, _irq_saved_context_pointer
	STR R14, [R12]

	LDMFD R13!, {R12, R14}
	MOV PC, R14

	;STMFD R13, {R0-R8}	;backup
	;MOV R1, R13
	;ADD R13, R13, #36	;9*4 (9 registers backed up)

	;load first 7 registers from stack
	;LDMFD R1!, {R2-R8}	;R0, R1, R2, R3, R4, R5, R6

	;store first 7 registers
	;ADD R0, R0, #28		;7 Registers each 4 byte long (7*4)
	;STMFD R0, {R2-R8}

	;load last 7 registers from stack
	;LDMFD R1, {R2-R8}	;R7, R8, R9, R10, R11, R12, R14

	;store last 7 registers
	;ADD R0, R0, #28
	;STMFD R0, {R2-R8}

	;LDMFD R13!, {R0-R8}	;load backuped registers
	;ADD R13, R13, #56	;remove all saved registers from stack

	;MOV PC, R14
