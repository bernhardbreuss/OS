/*
 * ipc.c
 *
 *  Created on: May 9, 2013
 *      Author: Stephan
 */

#include <ipc.h>
#include <process.h>
#include "../process/process_manager.h"
#include "../../hal/generic/mmu/mmu.h"
#include <inttypes.h>

static void copy_msg(Process_t* src, Process_t* dst) {
	src->ipc.msg->source = src->pid;
	src->ipc.msg->destination = dst->pid;
	memcpy(dst->ipc.msg, src->ipc.msg, sizeof(message_t));
}

int8_t ipc_handle_syscall(ProcessId_t o, uint8_t call_type, message_t* msg) {
	Process_t* src = process_manager_current_process;
	Process_t* dst = NULL;

	if (call_type == IPC_RECEIVE_ASYNC && src->ipc.sender.head == NULL) {
		return IPC_NOTHING_RECEIVED;
	}

	if (o == PROCESS_STDIN) {
		o = src->stdin;
	} else if (o == PROCESS_STDOUT) {
		o = src->stdout;
	}

	if (o == PROCESS_INVALID_ID) {
		return IPC_OTHER_NOT_FOUND;
	} else if ((call_type & IPC_RECEIVE) != IPC_RECEIVE || o != PROCESS_ANY) { /* allow ANY only on receive */
		dst = process_manager_get_process_byid(o);

		if (dst == NULL) {
			return IPC_OTHER_NOT_FOUND;
		}
	}

	src->ipc.other = o;
	src->ipc.msg = mmu_get_physical_address(src->page_table, msg);

	/* logger_debug("IPC: %u src=%i:%s dst=%i:%s", call_type, src->pid, src->name, o, (dst != NULL) ? dst->name : "<ANY>"); */

	switch (call_type) {
		case IPC_SEND:
		case IPC_SENDREC: /* SEND is falling through here */
			_disable_interrupts();
			if (dst->state == PROCESS_ZOMBIE) {
				_enable_interrupts();
				return IPC_DEAD;
			}
			if (dst->state == PROCESS_BLOCKED && dst->ipc.call_type == IPC_RECEIVE && (dst->ipc.other == src->pid || dst->ipc.other == PROCESS_ANY)) {
				_enable_interrupts();
				/* both process are now BLOCKED */
				copy_msg(src, dst);
				dst->ipc.call_type = IPC_NOOP;
				process_manager_set_process_ready(dst);
			} else {
				if (dst->ipc.call_type & IPC_SEND) {
					return IPC_DEADLOCK;
				}

				src->ipc.call_type = IPC_SEND;

				/* place msg for later delivery into the IPC queue of the destination */
				linked_list_add(&dst->ipc.sender, src); /* TODO: check for too less memory */

				/* pause process */
				process_manager_block_current_process();
				process_manager_run_process(dst);
				_enable_interrupts();
			}

			/* msg delivered, falling through receive except for send only */
			if (call_type == IPC_SEND) {
				break;
			}

		case IPC_RECEIVE: /* SENDREC and SEND are falling through here */
		case IPC_RECEIVE_ASYNC:
			_disable_interrupts(); /* TODO: check that process is not dead */
			if (dst != NULL) {
				if (dst->state == PROCESS_ZOMBIE) {
					_enable_interrupts();
					return IPC_DEAD;
				}
			} else {
				/* receiving from ANY, maybe someone is already sending to this process */
				linked_list_node_t* node;
				do {
					node = linked_list_pop_head(&src->ipc.sender);
					if (node != NULL) {
						dst = node->value;
						free(node);
					}
				} while (dst != NULL && dst->state == PROCESS_ZOMBIE);
			}
			if (dst != NULL && dst->state == PROCESS_BLOCKED && (dst->ipc.call_type & IPC_SEND) == IPC_SEND && dst->ipc.other == src->pid) {
				/* both process are now BLOCKED */
				_enable_interrupts();

				/* remove sender out of sender list */
				if (o != PROCESS_ANY) {
					linked_list_node_t* node = src->ipc.sender.head;
					while (node != NULL) {
						Process_t* p = node->value;
						if (p->pid == o) {
							linked_list_remove(&src->ipc.sender, node);
							break;
						}

						node = node->next;
					}
				}

				copy_msg(dst, src);
				dst->ipc.call_type = IPC_NOOP;
				process_manager_set_process_ready(dst);
			} else {
				if (dst != NULL && dst->ipc.call_type == IPC_RECEIVE) {
					return IPC_DEADLOCK;
				}

				src->ipc.call_type = IPC_RECEIVE;

				/* wait for msg delivery */
				process_manager_block_current_process();
				process_manager_run_process(dst);
				if (dst->state == PROCESS_ZOMBIE) {
					_enable_interrupts();
					return IPC_DEAD;
				}
				_enable_interrupts();

				/* msg received */
			}
			break;
	}

	return IPC_OK;
}
