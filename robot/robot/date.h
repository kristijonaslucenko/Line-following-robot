/*
 * date.h
 *
 * Created:  24/04/2015 15:37:40
 *  Author: Alexandru
 */ 


#ifndef DATE_H_
#define DATE_H_
#include "task.h"

typedef struct
{
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} date_t;

extern volatile date_t date;

void clock_tick(void);

void get_date_to_string(char *date_9b);
uint32_t get_date_to_int(void);

#endif /* DATE_H_ */