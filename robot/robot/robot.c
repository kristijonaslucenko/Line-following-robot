/*
* Car_Charger_Station.c
*
* The robot.c file is in charge of scheduling all the functionality
* that needs to be handled by the client.
*
* Created: 21/04/2015 19:37:11
*  Author: Administrator
*/



#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "isr.h"
#include "usart.h"
#include "task.h"
#include "common.h"
#include "date.h"
#include "com_prot.h"
#include "motor.h"
#include "adc.h"
#include "control_logic.h"
#include "led.h"

#define start(x){do_handler = true; x=true;}

// 1 represents 10 ms
#define CLOCK_INTERVAL				100
#define COMM_PROT_INTERVAL			10
#define MOTOR_INTERVAL				2
#define LED_INTERVAL				13
#define ADC_INTERVAL				10
#define SEND_ADC_VALUE_INTERVAL		50
#define SEND_SENSOR_INTERVAL		255u

timer_t test;

uint8_t led_int = 15;

void set_pid_int(task_t *task)
{
	led_int=task->data.value;
}

/**
* \brief Represents a scheduler implementation the scheduler ticking every 100ms.
*
* \author Alexandru
*
* \param
*
* \return int
*/
int main(void)
{
	uint8_t clock_timer					= CLOCK_INTERVAL;
	uint8_t com_prot_timer				= COMM_PROT_INTERVAL;
	uint8_t adc_timer					= ADC_INTERVAL;
	uint8_t motor_timer					= MOTOR_INTERVAL;
	uint8_t led_timer					= LED_INTERVAL;
	uint8_t send_adc_value_timer		= SEND_ADC_VALUE_INTERVAL;
	uint8_t send_sensor_timer			= SEND_SENSOR_INTERVAL;

	bool do_handler				= false;
	bool run_clock				= false;
	bool run_com_prot			= false;
	bool run_adc				= false;
	bool run_send_adc_value		= false;
	bool run_motor				= false;
	bool run_led				= false;
	bool run_send_sensor		= false;
	
	
	LED_PIN_init();
	led_off();
	led_on();
	
	status.byte[0]=0;
	adc_measurement_init();
	USART_init();
	timer1_init();
	recive_task_init();
	motors_init();
	led_init();
	
	enable_features.adc=false;
	enable_features.send_adc_value=false;
	enable_features.send_sensor_values=false;
	
	control = &state_idle_control_logic;
	//status.system.connected = true;
	
	sei();
	
	while(1)
	{
		if(system_tic == true) // 10 millisecond has passed
		{
			toggle_led();
			system_tic = false;
			if(--clock_timer == 0)
			{
				clock_timer = CLOCK_INTERVAL;
				start(run_clock);
			}
			if(--com_prot_timer == 0)
			{
				com_prot_timer = COMM_PROT_INTERVAL;
				start(run_com_prot);
			}
// 			if(--motor_timer == 0)
// 			{
// 				motor_timer = MOTOR_INTERVAL;
// 				start(run_motor);
// 			}
//  			if(--led_timer == 0)
//  			{
//  				led_timer = led_int;
//  				start(run_led);
//  			}
 			if( --adc_timer == 0 && enable_features.adc == true )
			
 			{
 				adc_timer = ADC_INTERVAL;
 				start(run_adc);
 			}
 			if(enable_features.send_adc_value == true && --send_adc_value_timer == 0)
 			{
 				send_adc_value_timer = SEND_ADC_VALUE_INTERVAL;
 				start(run_send_adc_value);
 			}
//  			if(--send_sensor_timer == 0)
//  			{
//  				send_sensor_timer = SEND_SENSOR_INTERVAL;
//  				start(run_send_sensor);
//  			}
		
		}
		if(do_handler)/*get_line_error();*/
		{
			do_handler = false;
			if (run_clock)
			{
				run_clock = false;
				clock_tick();
			}
			if (run_com_prot)
			{
				run_com_prot = false;
				com_prot_task();
			}
// 			if (run_motor)
// 			{
// 				run_motor = false;
// 				motor_handler();
// 				get_line_error();
// 			}
// 				
//  			if (run_led)
//  			{
//  				run_led = false;
//  				//get_line_error();
//  			}
 			
  			if (run_adc)
  			{
  				run_adc = false;
  				handleMeasurement();
  			}
  			
  			if (run_send_adc_value)
  			{
  				run_send_adc_value = false;
  				send_adc_value_to_pc();
  			}			
// 		
//  			if (run_send_sensor)
//   			{
//   				run_send_sensor = false;
//   				send_sensor_values();
//   			}
// 
// 			if (state_changed||new_data_available||tmr_exp(&state_timer))//can be further optimised
// 			{
//  				state_changed=false;
				(*control)();
//  			}
		}
		
		
	}
	return 1;
}