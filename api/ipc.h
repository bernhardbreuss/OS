/*
 * IPC.h
 *
 *  Created on: 09.05.2013
 *      Author: Stephan
 */

#ifndef IPC_H_
#define IPC_H_

#include <inttypes.h>
#include "device.h"
#include "message.h"

#define IPC_NOOP			0
#define IPC_SEND 			1	/* 0001:blocking send */
#define IPC_RECEIVE 		2	/* 0010:blocking receive */
#define IPC_SENDREC 		3	/* 0011:blocking SEND + RECEIVE */
#define IPC_RECEIVE_ASYNC	6   /* 0110:nonblocking receive */

#define IPC_OK					 0
#define IPC_OTHER_NOT_FOUND		-1
#define IPC_DEADLOCK			-2 /* TODO: deadlocks should be resolved by killing both processes in ipc.c */
#define IPC_DEAD				-3
#define IPC_NOTHING_RECEIVED	-4

int8_t ipc_syscall_device(Device_t device, uint8_t call_type, message_t* msg);

#pragma SWI_ALIAS(ipc_syscall, 0);
int8_t ipc_syscall(ProcessId_t dst, uint8_t call_type, message_t* msg);

#endif /* IPC_H_ */
