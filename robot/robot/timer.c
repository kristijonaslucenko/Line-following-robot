/**
* timer.c
*
* The usart.c file is in charge of initializing timer1 and setting up delay functionalities.
*
*  Created: 21/04/2015 19:37:11
*  Author: Alexandru Savin
*/

#include <avr/io.h>
#include <stdbool.h>
#include "isr.h"
#include "timer.h"
#include "common.h"


volatile uint32_t milliseconds;

/**
 * \brief Setup TIMER1 for 1 millisecond tic on TIMER1_COMPA_vect.
 *
 * \author Alexandru
 *
 * \param 
 * 
 * \return void
 */
void timer1_init(void)
{
	//normal port operation
	TCCR1A= 0;
	
	//CS12:0: Clock Select clk/8 From prescaler
	//WGM13:2: CTC mode on OCR1A
	TCCR1B|=(1<<CS11)|(1<<WGM12);
	
	//interrupt fires every 1millisecond
	//1/(10000000/8)*1250==1millisecond 1250=0x4E2
	OCR1A = 0x4E2;
	
	//OCIE1A: Timer/Counter1, Output Compare A Match Interrupt Enable
	TIMSK|=(1<<OCIE1A);
}

/**
* \brief Function used for setting up a timer_t type variable from the current millisecond offset to a delay.
*
* NOTE!! has to be a global variable or tmr_start has to be used before tmr_exp.
*
* Ex of use:
* timer_t delay_timer; NOTE!!! has to be global/static
* tmr_start(&delay_timer, dealy_in_milliseconds);
* if(tmr_exp(&delay_timer))
*   do something;
*
* \author Alexandru
*
* \param timer Used to specify a pointer to a timer_t type variable.
* \param delay Used to specify a delay in milliseconds.
*
* \return void
*/
void tmr_start(timer_t *timer, uint32_t delay_in_ms)
{
	timer->timer_start = milliseconds;
	timer->timer_expires = milliseconds + delay_in_ms; // (timer wraps around after ~ 50 days)
	timer->do_once = false;//TODO refactor to execute on false
}


/**
* \brief The function verifies if the delay has passed.
*
* \author Alexandru
*
* \param timer Used to specify a pointer to a timer_t type variable.
*
* NOTE!! has to be a global variable or tmr_start has to be used before tmr_exp.
*
* Ex of use:
* timer_t delay_timer; NOTE!!! has to be global/static
* tmr_start(&delay_timer, dealy_in_milliseconds);
* if(tmr_exp(&delay_timer))
*   do something;
*
* \return bool Returns true if the delay has passed.
*/
bool tmr_exp(timer_t *timer)
{
	uint32_t timer_start = timer->timer_start;
	uint32_t timer_expires  = timer->timer_expires; // making a copy of the timer var for safety 

 	if (timer->do_once == false) //check if the timer has been evaluated is set to false so no initialization of the timer is needed if the timer is global/static
 	{		
		if (timer_expires > timer_start) //check if the milliseconds variable has overlapped(once every 50 days)
		{//timer didn't overlap
			if((milliseconds >= timer_expires) || (milliseconds < timer_start)) //check timer boundaries 
			{//  check if milliseconds passed timer limit	     check if milliseconds overlapped		
				timer->do_once=true; //ensure that the timer is evaluated once only
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (timer_expires < timer_start)
		{//timer overlapped
			if((milliseconds < timer_start) && (milliseconds >= timer_expires))
			{// 	   check if milliseconds overlapped		 check if milliseconds passed timer limit
				timer->do_once = true;
				return true;
			}
			else
			{
				return false;
			}
		}
 		else //either the timer_expires == timer_start or both of them are 0 this is the case when the timer_t pointer is not initialised
 		{
     		timer->do_once = true;
 			return true; // timer_expires == timer_start
 		}
	}
 	else 
 	{
 		return false;
 	}
}