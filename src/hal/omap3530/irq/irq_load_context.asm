	.global irq_load_context

	.global irq_saved_context_pointer
_irq_saved_context_pointer: .field irq_saved_context_pointer, 32

	.global stackIRQ
_stackIRQ: .field stackIRQ, 32

irq_load_context:
	LDR R14, _irq_saved_context_pointer
	LDR R13, [R14]
	LDMDB R13, {R0-R12, R14}^
	SUB R13, R13, #64 ; 60 = 15*4 because writeback isn't possible + 4 = PC_user of new context
	LDMIB R13, {R13}^
	STR R13, [R14]
	LDR R14, [R13]

	LDR R13, _stackIRQ
	MOVS PC, R14
