/*
 * process_manager_test.h
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#ifndef PROCESS_MANAGER_TEST_H_
#define PROCESS_MANAGER_TEST_H_

/**
 * Try to add (MAX_PROCESSES + 1) to the ProcessManager.
 * This test validates that the last Process can not be added to the ProcessManager.
 *
 * The ProcessId of each Process is the array position in ProcessManager.processSlots pointer array.
 * If a Process is added successfully to the manager, its ProcessId is set to the index of the free slot the Process has been
 * added to.
 * This test also validates this behavior.
 *
 * @return 1 on success, error code on failure
 */
int test_process_manager_1(void);

#endif /* PROCESS_MANAGER_TEST_H_ */
