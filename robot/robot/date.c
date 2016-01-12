/*
 * date.c
 *
 * The date.c file is in charge of keeping track of the system clock.
 *
 *  Created: 24/04/2015 15:37:40
 *  Author: Alexandru Savin
 */

#include <avr/io.h>
#include <stdbool.h>
#include "date.h"
#include "task.h"
#include "common.h"

volatile date_t date;



/**
 * \brief Increments the seconds and checks if all the date related proprieties are valid.
 *
 * \author Alexandru
 *
 * \param 
 * 
 * \return void
 */
void clock_tick(void)
{
	if (++date.second == 60)
	{
		date.second =0;
		if (++date.minute == 60)
		{
			date.minute =0;
			if (++date.hour == 24)
			{
				date.hour =0;				
			}
		}
	}
}

/**
 * \brief Decodes the date struct into a string and copy's it to the passed var.
 *
 * \author Alexandru
 *
 * \param date_9b Used to specify  a 21bit char buffer.
 * 
 * \return void
 */
void get_date_to_string(char *date_9b)
{
	if (get_array_len(date_9b)==9)
	{
		date_9b[0]= date.hour/10+48;
		date_9b[1]= date.hour%10+48;
		date_9b[2]= ':';
		date_9b[3]= date.minute/10+48;
		date_9b[4]= date.minute%10+48;
		date_9b[5]= ':';
		date_9b[6]= date.second/10+48;
		date_9b[7]= date.second%10+48;
		date_9b[8]= '\n';
	} 
	else
	{
		date_9b[0]= 'B';
		date_9b[1]= 'U';
		date_9b[2]= 'F';
		date_9b[3]= 'F';
		date_9b[4]= ' ';
		date_9b[5]= 'O';
		date_9b[6]= 'V';
		date_9b[7]= 'F';
		date_9b[8]= '\n';
	}
    
}


/**
 * \brief 
 * 
 * \param 
 * 
 * \return uint32_t
 */
uint32_t get_date_to_int(void) 
{
	uint32_t date_u32=date.hour*10000;//245959
	date_u32 += date.minute*100;
	date_u32 += date.second;
	return date_u32;    
}