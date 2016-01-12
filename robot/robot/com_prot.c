/*
* com_prot.c
*
* The com_prot.c file is in charge of the communication layer of the client,
* servicing the pending tasks and the received once.
*
*  Created: 24/04/2015 15:37:40
*  Author: Alexandru Savin
*/

#include <stdlib.h>
#include "task.h"
#include "timer.h"
#include "common.h"
#include "date.h"
#include "usart.h"
#include "com_prot.h"
#include "motor.h"
#include "led.h"



/**
* \brief Used to initialize the communication.
* NOTE!!! The timer and USART functionalities have to be initialized prior to this function call.
*
* \author Alexandru
*
* \param
*
* \return void
*/
void communication_init(task_t *task)
{
	if (task->data.value == 0)
	{
		task_t comm_init = {.data.command = ACK_INIT_CONN, .data.value = get_task_number()};
		status.system.connected = true;
		clear_task_fifo();
		add_task(&comm_init);
	}
}

/**
* \brief Handles the message for the STOP command.
*
* \author Alexandru
*
* \param task Used to specify a pointer to a specific received task.
*
* \return void
*/
void stop(task_t *task)
{
	if (task->data.value == 0)
	{
		status.system.connected = false;
	}
}


/**
* \brief Handles the ACK message for the PING command.
*
* \author Alexandru
*
* \param task Used to specify a pointer to a specific received task.
*
* \return void
*/
void ping(void)
{
	
	read_switches();
	task_t ping = {.data.command = PING, .data.value = led.array/*get_task_number()*/};	
	add_task(&ping);
}


/**
* \brief This function handles all the communication between the server and the MCU.
*
* \author Alexandru
*
* \param
*
* \return void
*/
void com_prot_task(void)
{
	static task_t *tx_task;
	static timer_t ping_tmr;
	
	if (status.system.connected==true)
	{
		if (tmr_exp(&ping_tmr)) //send ping message every sec
		{
			tmr_start(&ping_tmr,SEC1);
			ping();
			led_off();
		}
		if (status.system.sending_task == false) //check if a transmission is in progress
		{
			tx_task = delete_task();
			
			if (tx_task != NULL) // manipulation on a null pointer results in memory damage
			{
				if (tx_task->data.command == STRING)
				{
					USART_transmit_string(tx_task->data.str);
				}
				else
				{
					USART_transmit_command(tx_task);
				}
			}
		}
	}
	if (status.system.task_received==true)
	{
		status.system.task_received=false;
		do_task[usart_rx_task.data.command](&usart_rx_task);
	}
}

