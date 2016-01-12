/**
* isr.c
*
* The isr.c file is in charge of servicing the ISR routines.
*
*  Created:  24/04/2015 15:37:40
*  Author: Alexandru Savin
*/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include "usart.h"
#include "isr.h"
#include "timer.h"
#include "common.h"
#include "motor.h"
#include "adc.h"

#define BREAK_COUNT 20
#define BREAK_COUNT_C 2
#define BREAK_FORCE 255
#define FAST_BRAKE -2

//#define Kp 110
uint8_t Kp = 4;
uint8_t Kd = 65;
#define KpBreak 1//error is always <0 when breaking therfore it needs to be converted to abs
//#define aplie_Kp(x) ((x*Kp)/64)
#define aplie_Kd(x) (((x)*Kd)/64)
#define aplie_Kp(x) (((x)*Kp))

/**
* \brief  Real-time motor Kp adjustments  has to be placed in data.u8[3]
*
* \param task
*
* \return void
*/
void set_Kp(task_t *task)
{
	Kp = task->data.u8[3];
	
	task_t pk_task = {.data.command = PID_KP, .data.value = Kp};
	add_task(&pk_task);
}
/**
* \brief Real-time motor Kd adjustments value will be divided by 64 has to be placed in data.u8[3]
*
* \param task
*
* \return void
*/
void set_Kd(task_t *task)
{
	Kd = task->data.u8[3];
	
	task_t pk_task = {.data.command = PID_KP, .data.value = Kd};
	add_task(&pk_task);
}


volatile status_t status;
volatile enable_features_t enable_features;
volatile uint32_t system_tic;
volatile int16_t p_r, p_l;
/**
* \brief Adds the received data to the rx_buffer when the frame is received a flag is raised.
*			When the frame is ready the value bites are swapped.
*
* \author Alexandru
*
* \param Vector for the interrupt.
*
* \return
*/
ISR(USART_RXC_vect)
{
	static uint8_t rx_index = 0;
	usart_rx_task.buffer[rx_index]=get_usart();
	if(++rx_index>get_array_len(usart_rx_task.buffer)-1)
	{
		rx_index = 0;
		status.system.task_received=true;
	}
}

/**
* \brief Copies the tx_buffer byte by byte to the U SART module buffer.
*
* \author Alexandru
*
* \param Vector for the  interrupt.
*
* \return
*/
ISR(USART_UDRE_vect)
{
	static uint8_t tx_index = 0;
	static char *string;
	
	if (usart_tx_task.data.command==STRING)
	{
		
		if (tx_index==0)
		{
			//string = usart_tx_task.data.str;
		}
		if (tx_index++ < 5)
		{
			//set_usart(usart_tx_task.buffer[tx_index]);
		}
		else
		{
			set_usart(*string);
			if(!*++string){
				tx_index=0;
				disable_uart_transmision();
				status.system.sending_task = false;
			}
			
		}
	}
	else
	{
		set_usart(usart_tx_task.buffer[tx_index]);
		if (++tx_index>get_array_len(usart_tx_task.buffer)-1)
		{
			tx_index = 0;
			
			disable_uart_transmision();
			status.system.sending_task = false;
		}
	}
}

/**
* \brief Increments the global millisecond variable that is needed for the timer.c module and
*			facilitates a system tick(10ms) used for scheduling purposes in the main loop.
*	motor controller is also implemented in here
*
* \author Alexandru
*
* \param Vector for the  interrupt.
*
* \return
*/

ISR(TIMER1_COMPA_vect)
{
	static uint8_t pulse_timer = 0,sys_timer = 0;
	static int16_t l_m,r_m;
	static int16_t l_error, r_error;
	
	if (++pulse_timer>7)// if happens to often can be increased up to 16ms
	{
		pulse_timer = 0;
		
		l_motor.error = l_motor.ref_pulses - p_l;
		r_motor.error = r_motor.ref_pulses - p_r;
		l_motor.pulse_count+=p_l;
		r_motor.pulse_count+=p_r;
		p_r = 0;
		p_l = 0;
		
		if(l_motor.ref_pulses!=0)
		{
			l_motor.do_once = true;
			if (l_motor.r_dir == FORWARD)
			{
				set_lf();
			}
			else
			{
				set_lb();
			}
			l_m+=aplie_Kp(l_motor.error)+aplie_Kd(l_motor.error-l_error);
			if (l_m>255)
			{
				l_m=255;
			}
			else if (l_m<1)
			{
				l_m=0;
			}
			set_left_m(l_m);
 			if (l_motor.error<0)
 			{
 				set_ld();
 			}
		}
		else
		{
			if (l_motor.do_once&&l_motor.error!=0)
			{
				l_motor.do_once = false;
				l_motor.breaking = ON;
				if (l_motor.finished_corner == true)
				{
					l_motor.finished_corner =false;
					l_motor.break_count = BREAK_COUNT_C;
				}
				else
				{
					l_motor.break_count = BREAK_COUNT;
				}
			}
			if (l_motor.breaking == ON)
			{
				if (l_motor.r_dir==FORWARD)
				{
					set_lb();
				}
				else
				{
					set_lf();
				}
				set_left_m(BREAK_FORCE);
				if (--l_motor.break_count<1)
				{
					l_motor.breaking = OFF;
				}
			}
			else
			{
				set_ls();
				if (l_motor.error!=0)
				{
					set_left_m(255);
				}
				else
				{
					set_left_m(0);
				}
			}
		}
		
		if(r_motor.ref_pulses!=0)
		{
			r_motor.do_once = true;
			if (r_motor.r_dir == FORWARD)
			{
				set_rf();
			}
			else
			{
				set_rb();
			}
			r_m+=aplie_Kp(r_motor.error)+aplie_Kd(r_motor.error-r_error);
			if (r_m>255)
			{
				r_m=255;
			}
			else if (r_m<1)
			{
				r_m=0;
			}
			set_right_m(r_m);
 			if (r_motor.error<0)
 			{
 				set_rd();
 			}
		}
		else
		{
			if (r_motor.do_once&&r_motor.error!=0)
			{
				r_motor.do_once = false;
				r_motor.breaking = ON;
				if (r_motor.finished_corner == true)
				{
					r_motor.finished_corner =false;
					r_motor.break_count = BREAK_COUNT_C;
				}
				else
				{
					r_motor.break_count = BREAK_COUNT;
				}
			}
			if (r_motor.breaking == ON)
			{
				if (r_motor.r_dir==FORWARD)
				{
					set_rb();
				}
				else
				{
					set_rf();
				}
				set_right_m(BREAK_FORCE);
				if (--r_motor.break_count<1)
				{
					r_motor.breaking = OFF;
				}
			}
			else
			{
				set_rs();
				if (r_motor.error!=0)
				{
					set_right_m(255);
				}
				else
				{
					set_right_m(0);
				}
			}
		}
		l_error = l_motor.error;
		r_error = r_motor.error;
	}
	
	if (++sys_timer>9)
	{
		sys_timer = 0;
		system_tic = true;
	}
	milliseconds++;
}

/**
* \brief Counts the pulses on the right motor
*
* \author Alexandru
*
* \param Vector for the  interrupt.
*
* \return
*/
ISR(INT0_vect){
	int0_toggle_edge();
	p_r++;
}

/**
* \brief Counts the pulses on the left motor
*
* \author Alexandru
*
* \param Vector for the  interrupt.
*
* \return
*/
ISR(INT1_vect){
	int1_toggle_edge();
	p_l++;
}


static u16_union adc_value;
ISR(ADC_vect) {
	
	get_adc(adc_value);
	if (first_channel){
		result0 = (adc_value.w * VSTEP) / 100;
		ADMUX&= 0xf8;
		ADMUX|= 0x07;
		ADCSRA |= (1<<ADSC);
		first_channel=false;
		} else {
		result1 = (adc_value.w * VSTEP) / 100;
		conversionIsInProgress = false;
		new_data_available = true;
		new_data_available_to_transmit = true;
	}
}
