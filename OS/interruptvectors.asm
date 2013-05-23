	.global udef_handler
	.global swi_handler
	.global pabt_handler
	.global dabt_handler
	.global irq_handler

	.sect ".interruptvectors"
		; See OMAP35x.pdf Page 3437 Table 23-10. RAM Exception Vectors
		B udef_handler	; Interrupt Undefined		0x4020FFC8
		B swi_handler	; Interrupt SWI				0x4020FFCC
		B pabt_handler	; Interrupt Prefetch Abort	0x4020FFD0
		B dabt_handler	; Interrupt Data Abort		0x4020FFD4
		.word 0			; Unused					0x4020FFD8
		B irq_handler	; Interrupt IRQ				0x4020FFDC
		.word 0			; Interrupt FIQ				0x4020FFE0
