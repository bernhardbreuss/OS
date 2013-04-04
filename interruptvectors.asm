	.global _c_int00
	.global swi_handler

	.sect ".interruptvectors"
		; See OMAP35x.pdf Page 3437 Table 23-10. RAM Exception Vectors
		B _c_int00		; Interrupt Reset			0x4020FFC4
		.word 0			; Interrupt Undefined		0x4020FFC8
		B swi_handler	; Interrupt SWI				0x4020FFCC
		.word 0			; Interrupt Prefetch Abort	0x4020FFD0
		.word 0			; Interrupt Data Abort		0x4020FFD4
		.word 0			; Unused					0x4020FFD8
		.word 0			; Interrupt IRQ				0x4020FFDC
		.word 0			; Interrupt FIQ				0x4020FFE0
