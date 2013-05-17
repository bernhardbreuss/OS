/*
 * ipc.c
 *
 *  Created on: May 9, 2013
 *      Author: Stephan
 */

#include "ipc.h"
#include "../process.h"
#include "../process_manager.h"
#include "../../driver/driver_manager.h"

uint32_t ipc_syscall_device(Device_t device, uint8_t call_type, message_t* msg) {
	//get the process for this driver
	//the getter starts the process internally if not running currently
	ProcessId_t process_id = driver_manager_get_process(device);
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

//TODO: ask the manager
extern Process_t* processSlots[MAX_PROCESSES];
uint32_t ipc_handle_syscall(ProcessId_t o, uint8_t call_type, message_t* msg) {
	Process_t* src = process_manager_get_current_process();
	Process_t* dst = NULL;

	if (call_type != IPC_RECEIVE || o != PROCESS_ANY) { /* TODO: allow ANY only on receive? */
		dst = process_manager_get_process_byid(o);

		if (dst == NULL) {
			return IPC_OTHER_NOT_FOUND;
		}
	}

	src->ipc.call_type = call_type;
	src->ipc.other = o;
	src->ipc.msg = msg;

	switch (call_type) {
		case IPC_SEND:
		case IPC_SENDREC: /* SEND is falling through here */
			if (dst->state == PROCESS_BLOCKED && (dst->ipc.other == src->pid || dst->ipc.other == PROCESS_ANY)) {
				/* both process are now BLOCKED */

				if (dst->ipc.call_type & IPC_SEND) {
					return IPC_DEADLOCK;
				}

				copy_msg(src, dst);
				src->ipc.call_type = IPC_RECEIVE; /* prepare for receiving */
				dst->ipc.call_type = IPC_NOOP;
				dst->state = PROCESS_READY;
			} else {
				/* place msg for later delivery */
				src->state = PROCESS_BLOCKED;

				/* pause process */
				process_manager_block_current_process(dst->pid);
			}

			/* msg delivered, falling through receive except for send only */
			if (call_type == IPC_SEND) {
				src->ipc.call_type = IPC_NOOP;
				break;
			}

		case IPC_RECEIVE: /* SENDREC and SEND are falling through here */
			if (dst == NULL) {
				/* receiving from ANY, maybe someone is already sending to this process */

				_disable_interrupts(); /* TODO: is this really necessary? */
				int i;
				for (i = 0; i < MAX_PROCESSES; i++) {
					Process_t* p = processSlots[i];
					if (p != NULL && p->state == PROCESS_BLOCKED && p->ipc.other == src->pid) {
						/* found a process which is sending to this process */
						dst = p;
						_enable_interrupts();
						break;
					}
				}
			}
			if (dst != NULL && dst->state == PROCESS_BLOCKED && dst->ipc.other == src->pid) {
				/* both process are now BLOCKED */

				if (dst->ipc.call_type == IPC_RECEIVE) {
					return IPC_DEADLOCK;
				}

				copy_msg(dst, src);
				dst->state = PROCESS_READY;
			} else {
				/* wait for msg delivery */
				src->state = PROCESS_BLOCKED;
				_enable_interrupts();

				/* pause process */
				process_manager_block_current_process(dst != NULL ? dst->pid : INVALID_PROCESS_ID);

				/* msg received */
			}
			break;
	}

	return IPC_OK;
}
