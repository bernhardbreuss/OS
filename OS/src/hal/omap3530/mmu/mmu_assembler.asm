	.global mmu_ttbr_set0
	.global mmu_ttbr_set1
	.global mmu_start
	.global mmu_init_hal
	.global mmu_get_ifsr
	.global mmu_get_ifar
	.global mmu_get_dfsr
	.global mmu_get_dfar
	.global pabt_handler
	.global dabt_handler

mmu_ttbr_set0:
	STMFD R13!, {R2, R3}
	; ARM Architecture Reference Manula p. 1336 - Synchronization of changes of ASID and TTBR
	;Set TTBCR.PD0 = 1
	MRC p15, #0, R2, c2, c0, #2 ; Read Translation Table Base Control Register
	ORR R3, R2, #0x10 ; Bit 4
	MCR p15, #0, R3, c2, c0, #2 ; Write Translation Table Base Control Register

	ISB

	;Change ASID to new value
	MCR p15, #0, R1, c13, c0, #1 ; Write Context ID Register

	;Change Translation Table Base Register to new value
	MCR p15, #0, R0, c2, c0, #0

	ISB

	;Set TTBCR.PD0 = 0
	MCR p15, #0, R2, c2, c0, #2 ; Write Translation Table Base Control Register

	LDMFD R13!, {R2, R3}
	MOV PC, R14

mmu_ttbr_set1:
	MCR p15, #0, R0, c2, c0, #1
	MOV PC, R14

;c1 Control Register
;0xCFFFE7FF (Sinnvolle Maske)
;C bit = 0 --> caching disabled
;Z bit = 0 --> program flow prediction disabled
;I bit = 0 --> instruction caching disabled at all levels
;TRE   = 0 --> TEX remap disabled
;AFE   = 0 --> Access Flag Enable AP[0] behavior defined
mmu_start:
	STMFD R13!, {R0-R1}

	MRC p15, #0, R0, c1, c0, #0	; Read Control Register
	ORR R0, R0, #0x1			; enable MMU
	MCR p15, #0, R0, c1, c0, #0	; Write Control Register

	LDMFD R13!, {R0-R1}
	MOV PC, R14

mmu_init_hal:
	SUB R13, R13, #4
	STR R0, [R13]

	MOV R0, #0x1	; set N to 1
	MCR p15, #0, R0, c2, c0, #2 ; Write Translation Table Base Control Register

	; set Domain Access Control Register
	MOV R0, #0x1	; Domain 0 is Client
	MCR p15, #0, R0, c3, c0, #0 ; Write Domain Access Control Register

	LDR R0, [R13]
	ADD R13, R13, #4

	MOV PC, R14

mmu_get_ifsr:
	MRC p15, #0, R0, c5, c0, #1 ; Read Instruction Fault Status Register
	MOV PC, R14

mmu_get_ifar:
	MRC p15, #0, R0, c6, c0, #2 ; Read Instruction Fault Address Register
	MOV PC, R14

mmu_get_dfsr:
	MRC p15, #0, R0, c5, c0, #0 ; Read Data Fault Status Register
	MOV PC, R14

mmu_get_dfar:
	MRC p15, #0, R0, c6, c0, #0 ; Read Data Fault Address Register
	MOV PC, R14

pabt_handler:
	SUB R14, R14, #4
	STMFD R13!, {R0-R2, R14}

	MRC p15, #0, R0, c5, c0, #1 ; Read Instruction Fault Status Register
	MRC p15, #0, R1, c6, c0, #2 ; Read Instruction Fault Address Register

	B foobar

dabt_handler:
	SUB R14, R14, #8
	STMFD R13!, {R0-R1, R14}

	MRC p15, #0, R0, c5, c0, #0 ; Read Data Fault Status Register
	MRC p15, #0, R1, c6, c0, #0 ; Read Data Fault Address Register

foobar:
	STR R1, _mmu_asm_va
	ADR R1, _mmu_asm_va

	BL _mmu_handle_abort
	STR R0, _mmu_asm_size
	LDMFD R13!, {R0-R1, R14}

	CMP R0, #0
	BNE foobar_load

	MOVS PC, R14

foobar_load:
	CPS #0x1F
	STMFD R13!, {R0-R4, R14}

	CPS #0x17
	MOV R2, R14
	MRS R3, SPSR
	CPS #0x1F

	LDR R0, _mmu_asm_va
	LDR R1, _mmu_asm_size

	;enable interrupts
	MRS R4, CPSR
	BIC R4, R4, #0xC0	; enable interrupts
	MSR CPSR_c, R4

	BL loader_load

	MRS R0, CPSR
	BIC R0, R0, #0x1F	; clear mode bits
	ORR R0, R0, #0xD7	; abort mode, disable interrupts
	MSR CPSR_c, R0

	MSR SPSR_fsxc, R3
	MOV R14, R2

	CPS #0x1F
	LDMFD R13!, {R0-R4, R14}
	CPS #0x17
	MOVS PC, R14

	.global _mmu_handle_abort
	.global loader_load
_mmu_asm_va:	.field 0
_mmu_asm_size:	.field 0
