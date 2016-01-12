/*
* led.c
*
* Created: 24/04/2015 20:45:45
*  Author: SavinA
*/

#include <avr/io.h>
#include <stdint.h>

#include "led.h"
#include "motor.h"
#include "math_Q.h"
#include "task.h"
#include "common.h"
#include "control_logic.h"

//TODO: check if this works
#define read_switch(x)	(PINA & (1<<PA##x))
#define SW0 read_switch(0)
#define SW1 read_switch(1)
#define SW2 read_switch(2)
#define SW3 read_switch(3)
#define SW4 read_switch(4)
#define SW5 read_switch(5)
#define SW6 read_switch(6)
#define SW7 read_switch(7)

#define send_led_info(){task_t led_info = {.data.command = LED, .data.value = led.array};add_task(&led_info);}
#define send_pid(x){task_t pid_err = {.data.command = PID_ERROR, .data.value = x};add_task(&pid_err);}

#define send_left_m(x) {task_t m_info = {.data.command = MOTOR_L, .data.value = l_motor.rpm};add_task(&m_info);}
#define send_right_m(x){task_t m_info = {.data.command = MOTOR_R, .data.value = r_motor.rpm};add_task(&m_info);}

// #define Kp 16
// #define Ki 0
// #define Kd 0

static int16_t Kp=24, Ki=0, Kd=0;
static uint8_t err4=6, err5=9, err6=11;

#define aplie_Kd(x)(((x)*Kd)/2)
#define aplie_Ki(x)(((x)*Ki))
/*#define ERROR_STEP 50*/

#define REF_RPM 129
#define MAX_DEVIATION 200
#define MAX_I_DEVIATION 40

volatile led_t led;


/**
* \brief  Returns the line error
*
*
* \return in8_t Returns a value between -3 and 3 where -3 represents the biggest left error and 3 the biggest right error  0 no error 10 if no line //TODO: rephrase  check if switches react as they should
*/
void get_line_error(void)
{
	if (status.system.start_line)//TODO if java tool is used uncomment
	{
		static int16_t i_factor,last_error,d_factor,p_factor, pid;
		static int8_t error = 0;
		read_switches();
		switch (led.array)//- line on right +line on left
		{
			case 0b01100011 : //0
			{
				error = 0;
				i_factor = 0;
				toggle_led();
			}
			break;
			case 0b01100111 : //-1
			error = 1;
			break;
			case 0b01110011 : //1
			error = -1;
			break;
			case 0b01000111 : //-2
			error = 2;
			break;
			case 0b01110001 : //2
			error = -2;
			break;
			case 0b01001111 : //-3
			error = 3;
			break;
			case 0b01111001: //3
			error = -3;
			break;
			case 0b00001111: //-4
			error = err4;
			break;
			case 0b01111000: //4
			error = -err4;
			break;
			case 0b00011111: //-5
			error = err5;
			break;
			case 0b01111100: //5
			error = -err5;
			break;
			case 0b00111111: //-6
			error = err6;
			break;
			case 0b01111110: //6
			error = -err6;
			break;
			case 0b00000000:
// 			error = 0;
// 			i_factor = 0;
			
			break;
			case 0b01111111:
// 			error = 0;
// 			i_factor = 0;
			if (last_error==-6||last_error==6)
			{
				//TODO: add sate to go back
			}
			
			break;
			//more cases for the special lines
			default:
			{
				//error =0;
				
			}
			break;
		}
// 		if (error!=last_error)
// 		{
			p_factor = error*Kp;
			
			i_factor +=error;
			if (i_factor>MAX_I_DEVIATION)
			{
				i_factor=MAX_I_DEVIATION;
			}
			else if(i_factor<-MAX_I_DEVIATION)
			{
				i_factor=-MAX_I_DEVIATION;
			}

			d_factor =aplie_Kd(error-last_error);
			
			pid=p_factor+aplie_Ki(i_factor)+d_factor;
			
			if (pid>MAX_DEVIATION)
			{
				pid=MAX_DEVIATION;
			}
			else if (pid<-MAX_DEVIATION)
			{
				pid= -MAX_DEVIATION;
			}
// 			if(error<0)//- line on right +line on left
// 			{
// 				r_motor.rpm = 17;//r_motor.ref_rpm + pid; //decrees
// 				l_motor.rpm = 100;//l_motor.ref_rpm;
// 				//l_motor.rpm = l_motor.ref_rpm -pid/2;
// 			}
// 			else
// 			{
// 				l_motor.rpm = 17;//l_motor.ref_rpm - pid;//decrees
// 				r_motor.rpm = 100;//r_motor.ref_rpm;
// 				//r_motor.rpm = r_motor.ref_rpm + pid/2;
// 			}
			l_motor.rpm = l_motor.ref_rpm - pid;
			r_motor.rpm = r_motor.ref_rpm + pid;
			
	//}
	last_error = error;
		static uint8_t info_timer=10;//5*70ms = 350ms
		if(--info_timer==0)
		{
			info_timer=10;
			//send_led_info();
			send_left_m();
			send_right_m();
			//send_pid(pid);
			//task_t led_info  = {.data.command = PID_L_KP, .data.value = p_factor};add_task(&led_info);
			//task_t led_info1 = {.data.command = PID_L_KI, .data.value = i_factor};add_task(&led_info1);
			//task_t led_info2 = {.data.command = PID_L_KD, .data.value = d_factor};add_task(&led_info2);
		}
	}
}

void send_sensor_values(void) {
	read_switches();
	send_led_info();
}

void start_line(task_t *task)
{
	if (task->data.u8[3]>0)
	{
		status.system.start_line=1;
		l_motor.ref_rpm=task->data.u8[3]*2;
		r_motor.ref_rpm=task->data.u8[3]*2;
		l_motor.rpm=task->data.u8[3]*2;
		r_motor.rpm=task->data.u8[3]*2;
	}
	else
	{
		status.system.start_line=0;
		l_motor.ref_rpm=0;
		r_motor.ref_rpm=0;
		l_motor.rpm=0;
		r_motor.rpm=0;
	}
}

void set_l_Kp(task_t *task)
{
	Kp=task->data.u8[3];
	task_t led_info  = {.data.command = PID_L_KP, .data.value = Kp};add_task(&led_info);
}

void set_l_Ki(task_t *task)
{
	Ki=task->data.u8[3];
	task_t led_info  = {.data.command = PID_L_KI, .data.value = Ki};add_task(&led_info);
}

void set_l_Kd(task_t *task)
{
	Kd=task->data.u8[3];
	task_t led_info  = {.data.command = PID_L_KD, .data.value = Kd};add_task(&led_info);
}

void set_err4(task_t *task)
{
	err4=task->data.u8[3];
	task_t led_info  = {.data.command = PID_ERR4, .data.value = err4};add_task(&led_info);
}

void set_err5(task_t *task)
{
	err5=task->data.u8[3];
	task_t led_info  = {.data.command = PID_ERR5, .data.value = err5};add_task(&led_info);
}

void set_err6(task_t *task)
{
	err6=task->data.u8[3];
	task_t led_info  = {.data.command = PID_ERR6, .data.value = err6};add_task(&led_info);
}

void set_err_p1(task_t *task)
{
	err4++;
	err5++;
	err6++;
	task_t led_info  = {.data.command = PID_ERR4, .data.value = err4};add_task(&led_info);
	task_t led_info1  = {.data.command = PID_ERR5, .data.value = err5};add_task(&led_info);
	task_t led_info2  = {.data.command = PID_ERR6, .data.value = err6};add_task(&led_info);
}

void set_pid(task_t *task)
{
	Kp=task->data.u8[0];
	Ki=task->data.u8[1];
	Kd=task->data.u8[2];
	task_t led_info  = {.data.command = PID_L_KP, .data.value = Kp};add_task(&led_info);
	task_t led_info1 = {.data.command = PID_L_KI, .data.value = Ki};add_task(&led_info1);
	task_t led_info2 = {.data.command = PID_L_KD, .data.value = Kd};add_task(&led_info2);
}

/**
* \brief initializes the pins for the light switches
*
* \param
*
* \return void
*/
void led_init(void)
{
	set_pin_as_input(A,0);
	set_pin_as_input(A,1);
	set_pin_as_input(A,2);
	set_pin_as_input(A,3);
	set_pin_as_input(A,4);
	set_pin_as_input(A,5);
	set_pin_as_input(B,0);
	l_motor.ref_rpm=129;
	r_motor.ref_rpm=129;
}