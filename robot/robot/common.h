/*
* common.h
*
* Created: 24/04/2015 15:37:40
*  Author: Administrator
*/


#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include "timer.h"
#include "task.h"

/*
* PIN name corresponding to the led
*/
#define LED_PIN PB2
/**
* \brief Toggles the LED that is present on port b pin 0.
*/
#define toggle_led()		{PORTB^=(1<<LED_PIN);}
/*
* Sets the LED on.
*/
#define led_on()			{PORTB&=~(1<<LED_PIN);}
/*
* Sets the LED off.
*/
#define led_off()			{PORTB|=(1<<LED_PIN);}
/*
* Inits the LED pin.
*/
#define LED_PIN_init()			{DDRB|=(1<<LED_PIN);}
/**
* \brief Used for copying task buffers to the UART, copies first 5 bits of a buffer.
*/
#define task_buffer_copy(paste_buffer,copy_buffer)		{paste_buffer[0]=copy_buffer[0]; paste_buffer[1]=copy_buffer[1]; paste_buffer[2]=copy_buffer[2]; paste_buffer[3]=copy_buffer[3]; paste_buffer[4]=copy_buffer[4]; paste_buffer[5]=copy_buffer[5]; paste_buffer[6]=copy_buffer[6]; paste_buffer[7]=copy_buffer[7]; paste_buffer[8]=copy_buffer[8];}
/**
* Decodes an uint value into a string.
* NOTE!!! Last char has to be manually set to 0 if string parsing wants to be used.
*/
#define uint_to_string(uint_nr,string_ptr, string_size)	{for(int i=0;uint_nr || i>string_size;i++){string_ptr[string_size-i-1]=uint_nr%10+48;	uint_nr/=10;}}

/**
* Used for debug porpoises it sends an INIT_CONN command with a desired value attached to it.
*/
task_t test_task;
#define debug_task(u32_value)							{test_task.data.command = INIT_CONN;	test_task.data.value = u32_value;	USART_transmit_command(&test_task);}

/**
* Returns the size of an buffer
*/
#define get_array_len(arrayName) (sizeof(arrayName) / sizeof((arrayName)[0]))

#define set_pin_as_input(port, pin_number)	{DDR##port&=~(1<<pin_number);}
#define set_pin_as_output(port, pin_number)	{DDR##port|=(1<<pin_number);}

/**
* Used for converting different types of values
*/
typedef union
{
	uint8_t b[4];
	uint16_t w[2];
	uint32_t dw;
} u32_union;

typedef union
{
	uint8_t b[2];
	uint16_t w;
} u16_union;

/**
* Encapsulates all the system status related flags in one place, for easy assignment and clearing.
*/
typedef union
{
	struct
	{
		uint8_t connected : 1;
		
		uint8_t sending_task : 1;
		uint8_t task_received : 1;

		uint8_t circle : 1;
		
		uint8_t start_line : 1;
		
		uint8_t wall : 1;
		
		uint8_t not_used6 : 1;
		uint8_t not_used7 : 1;
	} system;
	uint8_t byte[4];
} status_t;

void set_pid_int(task_t *task);

extern volatile status_t status;

typedef struct
{
	bool adc;
	bool send_adc_value;
	bool send_sensor_values;
	bool controller;
} enable_features_t;

volatile enable_features_t enable_features;

typedef enum {STATE_IDLE, STATE_FIND_TRACK, STATE_FIND_GO_A_BIT_MORE, STATE_FIND_LEFT_TURN, STATE_FOLLOW_TRACK_1, STATE_Y_INTERSECTION, STATE_FOLLOW_TRACK_2, STATE_WAIT_BEFORE_CORNER, STATE_APPROACH_CORNER, STATE_TURN_AFTER_FOUND_CORNER, STATE_GO_AHEAD_AFTER_TURN, STATE_GO_A_BIT_MORE, STATE_SECOND_LEFT_TURN, STATE_SECOND_GO_AHEAD, STATE_SECOND_GO_A_BIT_MORE, STATE_THIRD_LEFT_TURN, STATE_THIRD_GO_AHEAD, STATE_RIGHT_TURN, STATE_LAST_GO_AHEAD, STATE_FINISH, STATE_TAKE_OVER} state_t;

#endif /* COMMON_H_ */