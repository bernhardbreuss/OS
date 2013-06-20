/*
 * kernel.c
 *
 *  Created on: Jun 4, 2013
 *      Author: Bernhard
 */

#include "system.h"
#include "process/process_manager.h"
#include <ipc.h>
#include "string.h"
#include <ring_buffer.h>
#include "../platform/platform.h"
#include "../hal/generic/mmu/mmu.h"
#include "../hal/generic/irq/irq.h"

static message_t msg;

static uint8_t system_start_process(void) {
	Process_t* p = process_manager_start_process_bybinary((binary_t*)msg.value.data[1], PROCESS_PRIORITY_HIGH, &msg.value.buffer[sizeof(unsigned int) * 3]);
	if (p != NULL) {
		msg.value.data[1] = p->pid;
	} else {
		msg.value.data[1] = (unsigned int)PROCESS_INVALID_ID;

		ProcessId_t pid = msg.value.data[2];
		if (pid != PROCESS_INVALID_ID) {
			Process_t* other = process_manager_get_process_byid(pid);

			if (other != NULL) {
				other->stdin = pid;
				p->stdout = pid;
			}
		}
	}

	msg.value.data[0] = SYSTEM_OK;
	return 1;
}

static uint8_t system_find_process(void) {
	Process_t* p = process_manager_get_process_byname(&msg.value.buffer[sizeof(unsigned int)]);
	if (p == NULL) {
		msg.value.data[0] = SYSTEM_ERROR;
		return 1;
	} else {
		msg.value.data[1] = p->pid;
		msg.value.data[0] = SYSTEM_OK;
		return 1;
	}
}

static uint8_t system_start_command(void) {

	char* command = (char*)&msg.value.data[1];

	/* FIXME */
	ProcessId_t pid = 0; /* FIXME process_manager_start_process_bybinary... */

	msg.value.data[0] = SYSTEM_OK;
	msg.value.data[1] = pid;
	return 1;
}

static uint8_t system_end_process(void) {
	process_manager_end_process(msg.source, msg.value.data[1]);
	return 0;
}

static uint8_t mem_io_read(void){

	unsigned int* address;
	unsigned int address_data;
	int i;

	for( i = 1 ; i < msg.size; i++ ){
		address = (unsigned int*) msg.value.data[i];
		address_data = *address;
		msg.value.data[i] =  address_data;
	}

	msg.value.data[0] = SYSTEM_OK;
	return 1;
}

static uint8_t mem_io_write(void){

	unsigned int* address;
	int i;

	for( i = 1 ; i < msg.size; i += 2 ){
		address = (unsigned int*) msg.value.data[i+1];
		*address = msg.value.data[i];
	}

	msg.value.data[0] = SYSTEM_OK;
	return 1;
}

typedef struct {
	Process_t* process;
	ring_buffer_t* buffer;
	char* address;
	size_t size;
} user_interrupt_t;
static user_interrupt_t user_interrupts[INTCPS_IRQ_MAX_COUNT];

static void interrupt_handler(int id) {
	if (user_interrupts[id].process == NULL) {
		return;
	}

	if (user_interrupts[id].process->state == PROCESS_ZOMBIE) {
		return; /* TODO: remove interrupt register */
	}

	user_interrupt_t* u = &user_interrupts[id];
	mmu_activate_process(u->process);
	ring_buffer_put(u->buffer, u->address, u->size);

	mmu_activate_process(process_manager_current_process);
}

static uint8_t register_interrupt(void) {
	Process_t* p = process_manager_get_process_byid(msg.source);
	ring_buffer_t* buffer = (ring_buffer_t*)msg.value.data[1];

	int id = msg.value.data[2];
	user_interrupts[id].process = p;
	user_interrupts[id].buffer = buffer;
	user_interrupts[id].address = (char*)msg.value.data[3];
	user_interrupts[id].size = msg.value.data[4];

	irq_add_handler(id, &interrupt_handler);

	return 1;
}

void system_main_loop(void) {
	while (1) {
		int ipc = ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);
		if (ipc == IPC_OK) {
			uint8_t answer = 0;

			switch (msg.value.data[0]) {
			case SYSTEM_START_PROCESS:
				answer = system_start_process();
				break;
			case SYSTEM_FIND_PROCESS:
				answer = system_find_process();
				break;
			case SYSTEM_START_COMMAND:
				answer = system_start_command();
				break;
			case MEM_IO_READ:
				answer = mem_io_read();
				break;
			case MEM_IO_WRITE:
				answer = mem_io_write();
				break;
			case SYSTEM_END_PROCESS:
				answer = system_end_process();
				break;
			case REGISTER_INTERRUPT:
				answer = register_interrupt();
			default:
				answer = 1;
				msg.value.data[0] = SYSTEM_ERROR;
			}

			if (answer) {
				ipc_syscall(msg.source, IPC_SEND, &msg);
			}
		}
	}
}
