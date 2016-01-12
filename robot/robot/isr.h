/*
 * isr.h
 *
 * Created:  24/04/2015 15:37:40
 *  Author: Alexandru Savin
 */ 


#ifndef ISR_H_
#define ISR_H_
#include "task.h"
extern volatile uint32_t system_tic;

void set_Kp(task_t *task);
void set_Kd(task_t *task);

#endif /* ISR_H_ */