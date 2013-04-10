#ifndef PROCESS_MANAGEMENT_H_
#define PROCESS_MANAGEMENT_H_

#include "../prcoess.h"

#define TABLE_SIZE 10		// TODO: TABLE_SIZE? Maybe there is a better description of that...

/**
 * Describes an entry of the process management table.
 * Each process is represented via one entry in the table.
 */
typedef struct {
	Process_t* process;
	// TODO extend
	// Program counter (typedef?)
	// Stack pointer
	// Memory allocation
	// (status of its open files)
	// anything else?
} ProcessTableEntry_t;

typedef struct {
	ProcessTableEntry_t* processTable[TABLE_SIZE];
} ProcessTable_t;

void pm_init();

int pm_register_process(const ProcessManagementEntry_t* pme);

ProcessTableEntry_t* pm_get_process(int id);


#endif /* PROCESS_MANAGEMENT_H_ */
