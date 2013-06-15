/*
 * ipc.c
 *
 *  Created on: May 9, 2013
 *      Author: Stephan
 */

#include "ipc.h"
#include <process.h>
#include "../process/process_manager.h"
#include "../../hal/generic/mmu/mmu.h"

uint32_t ipc_syscall_device(Device_t device, uint8_t call_type, message_t* msg) {
	//get the process for this driver
	//the getter starts the process internally if not running currently
	ProcessId_t process_id = INVALID_PROCESS_ID; // TODO: driver_manager_get_process(device);
	if (process_id != INVALID_PROCESS_ID) {
		return ipc_syscall(process_id, call_type, msg); //send message via software interrupt
	} else {
		return IPC_OTHER_NOT_FOUND;
	}
}

void copy_msg(Process_t* src, Process_t* dst) {
	src->ipc.msg->source = src->pid;
	src->ipc.msg->destination = dst->pid;
	memcpy(dst->ipc.msg, src->ipc.msg, sizeof(message_t));
}

uint32_t ipc_handle_syscall(ProcessId_t o, uint8_t call_type, message_t* msg) {
	Process_t* src = process_manager_current_process;
	Process_t* dst = NULL;

	if (call_type != IPC_RECEIVE || o != PROCESS_ANY) { /* allow ANY only on receive */
		dst = process_manager_get_process_byid(o);

		if (dst == NULL) {
			return IPC_OTHER_NOT_FOUND;
		}
	}

	src->ipc.call_type = call_type;
	src->ipc.other = o;
	src->ipc.msg = mmu_get_physical_address(src->page_table, msg);
	mmu_map(process_manager_kernel_process.page_table, src->ipc.msg, src->ipc.msg);

	switch (call_type) {
		case IPC_SEND:
		case IPC_SENDREC: /* SEND is falling through here */
			_disable_interrupts(); /* TODO: Stephan, pls check */
			if (dst->state == PROCESS_BLOCKED && (dst->ipc.other == src->pid || dst->ipc.other == PROCESS_ANY)) {
				_enable_interrupts();
				/* both process are now BLOCKED */

				if (dst->ipc.call_type & IPC_SEND) {
					return IPC_DEADLOCK;
				}

				copy_msg(src, dst);
				src->ipc.call_type = IPC_RECEIVE; /* prepare for receiving */
				dst->ipc.call_type = IPC_NOOP;
				process_manager_set_process_ready(dst);
			} else {
				/* place msg for later delivery into the IPC queue of the destination */
				linked_list_add(&dst->ipc.sender, src);

				/* pause process */
				process_manager_block_current_process();
				process_manager_run_process(dst);
				_enable_interrupts();
			}

			/* msg delivered, falling through receive except for send only */
			if (call_type == IPC_SEND) {
				src->ipc.call_type = IPC_NOOP;
				break;
			}

		case IPC_RECEIVE: /* SENDREC and SEND are falling through here */
			_disable_interrupts(); /* TODO: Stephan, pls check */
			if (dst == NULL) {
				/* receiving from ANY, maybe someone is already sending to this process */

				linked_list_node_t* node = linked_list_pop_head(&src->ipc.sender);
				if (node != NULL) {
					dst = node->value;
					free(node);
				}
			} /* TODO: else check if sender is in sender list and remove if necessary */
			if (dst != NULL && dst->state == PROCESS_BLOCKED && dst->ipc.other == src->pid) {
				/* both process are now BLOCKED */
				_enable_interrupts();

				if (dst->ipc.call_type == IPC_RECEIVE) {
					return IPC_DEADLOCK;
				}

				copy_msg(dst, src);
				process_manager_set_process_ready(dst);
			} else {
				/* wait for msg delivery */
				process_manager_block_current_process();
				process_manager_run_process(dst);
				_enable_interrupts();

				/* msg received */
			}
			break;
	}

	src->ipc.call_type = IPC_NOOP;

	return IPC_OK;
}
