/*
 * IPC.h
 *
 *  Created on: 09.05.2013
 *      Author: Stephan
 */

#ifndef IPC_H_
#define IPC_H_

#define SEND 		1	/* 0001:blocking send*/
#define RECEIVE 	2	/* 0010:blocking receive*/
#define SENDREC 	3	/* 0011:blocking SEND + RECEIVE*/
#define NOTIFY 		4	/* 0100:blocking nonblocking notify*/
#define ECHO 		8	/* 1000:blocking echo a message*/

#endif /* IPC_H_ */
