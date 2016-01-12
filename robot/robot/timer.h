/*
 * timer.h
 *
 * Created:  24/04/2015 15:37:40
 *  Author: Administrator
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <stdint.h>
/*
 * Used with tmr_start, represents half second.
 */
#define SEC1_2	500
/*
 * Used with tmr_start, represents 1 second.
 */
#define SEC1	1000
/*
 * Used with tmr_start, represents 2 second.
 */
#define SEC2	2000
/*
 * Used with tmr_start, represents 3 second.
 */
#define SEC3	3000
/*
 * Used with tmr_start, represents 4 second.
 */
#define SEC4	4000
/*
 * Used with tmr_start, represents 5 second.
 */
#define SEC5	5000

/**
* \brief The structure is used to enable delay functionalities has to be used in the following way:
* 
* timer_t delay_timer; NOTE!!! has to be global/static
* tmr_start(&delay_timer, dealy_in_milliseconds);
* if(tmr_exp(&delay_timer))
*   do something;
*/
typedef struct 
{
    uint32_t timer_start;		//!< Start time of the timer
    uint32_t timer_expires;		//!< Alarm time.
    uint8_t do_once;			//!< Evaluate only once.
} timer_t;

extern volatile uint32_t milliseconds;

void timer1_init(void);
bool tmr_exp(timer_t *timer);
void tmr_start(timer_t *timer, uint32_t delay_in_ms);

#endif /* TIMER_H_ */