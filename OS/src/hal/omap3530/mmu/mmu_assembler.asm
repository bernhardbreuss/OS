	.global mmu_ttbr_set0
	.global mmu_ttbr_set1
	.global mmu_start
	.global mmu_init_hal
	.global pabt_handler
	.global dabt_handler

	; #includes
	.global process_context_save
	.global process_context_load
	.global _mmu_handle_abort
	.global process_manager_change_process

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

	MOV R0, #0x2	; set N to 1
	MCR p15, #0, R0, c2, c0, #2 ; Write Translation Table Base Control Register

	; set Domain Access Control Register
	MOV R0, #0x1	; Domain 0 is Client
	MCR p15, #0, R0, c3, c0, #0 ; Write Domain Access Control Register

	LDR R0, [R13]
	ADD R13, R13, #4

	MOV PC, R14

pabt_handler:
	SUB R14, R14, #4
	SUB R13, R13, #4
	STR R14, [R13]
	BL process_context_save
	ADD R13, R13, #4

	MRC p15, #0, R0, c5, c0, #1 ; Read Instruction Fault Status Register
	MRC p15, #0, R1, c6, c0, #2 ; Read Instruction Fault Address Register

	BL _mmu_handle_abort

	MOV R0, #0
	BL process_manager_change_process

	B process_context_load


dabt_handler:
	SUB R14, R14, #8
	SUB R13, R13, #4
	STR R14, [R13]
	BL process_context_save
	ADD R13, R13, #4

	MRC p15, #0, R0, c5, c0, #0 ; Read Data Fault Status Register
	MRC p15, #0, R1, c6, c0, #0 ; Read Data Fault Address Register

	BL _mmu_handle_abort

	MOV R0, #0
	BL process_manager_change_process

	B process_context_load
