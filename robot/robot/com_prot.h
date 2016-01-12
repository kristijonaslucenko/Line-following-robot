/*
 * com_prot.h
 *
 *  Created: 24/04/2015 15:37:40
 *  Author: Alexandru
 */ 


#ifndef COM_PROT_H_
#define COM_PROT_H_


void com_prot_task(void);

void communication_init(task_t *task);
void stop(task_t *task);


#endif /* COM_PROT_H_ */