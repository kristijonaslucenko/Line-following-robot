/**
* usart.c
* 
* The usart.c file is in charge of initializing the USART module and  
* contains functionality for sending task structures to the server.
*
*  Created: 21/04/2015 19:37:11
*  Author: Alexandru Savin
*/

#include <avr/io.h>
#include <stdbool.h>
#include "usart.h"
#include "task.h"
#include "common.h"
#include "date.h"


task_t usart_tx_task,usart_rx_task;

/**
* \brief Setting up the USART module full duplex 8 bit frame 19200bps.
*
* \author Alexandru
*
* \return void
*/
void USART_init(void)
{
    ///* Set baud rate*/
    //UBRRH = (unsigned char)(baud>>8);
    //UBRRL = (unsigned char)baud;
	
    
    
    //URSEL: Register Select
    //use data sheet or http://www.wormfood.net/avrbaudcalc.php to calculate
    UBRRH&=~(1<<URSEL);
    UBRRL = 129; //baud rate set to 9600bps //fosc/(8*baud)-1
    
    //U2X: Double the USART Transmission Speed
    UCSRA =(1<<U2X);
    
    //RXCIE: RX Complete Interrupt Enable
    //UDRIE: USART Data Register Empty Interrupt Enable
    //TXEN: Transmitter Enable
    //RXEN: Receiver Enable
    UCSRB|=(1<<RXCIE)|(1<<RXEN)|(1<<TXEN);
    
    //URSEL: Register Select
    //UCSZ1:0: Character Size 8bit frame size
    UCSRC|=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	PORTD |= (1<<PD0);
}


/**
* \brief Decodes the task structure into a series of bytes rearranging them for proper communication with the server and starts transmitting.
*
* \author Alexandru
*
* \param task Used to specify a pointer to a task, that needs to be transmitted.
*
* \return void
*/
void USART_transmit_command(task_t* task)
{
	status.system.sending_task=true;
	task->data.timestamp=get_date_to_int();
	task_buffer_copy(usart_tx_task.buffer,task->buffer);
	enable_uart_transmision();
}


/**
* \brief Used to send a string over usart
*
* \author Alexandru
*
* \param task Used to specify a pointer to a string, that needs to be transmitted.
*
* \return void
*/
void USART_transmit_string(char* string)
{
	task_t string_task = {.data.command = STRING, .data.str = string};
	status.system.sending_task=true;
	string_task.data.timestamp=get_date_to_int();
	task_buffer_copy(usart_tx_task.buffer,string_task.buffer);
	enable_uart_transmision();
}