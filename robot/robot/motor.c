/*
* motor.c
*
* Created: 21/04/2015 19:37:11
*  Author: SavinA
*/

/************************************************************************/
/* change Kp value                                                                     */
/************************************************************************/

/**
* \brief Kp for the motor controller
* the actual coefisient has to be multiplied by 128 and an integer value has to be provided
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "motor.h"
#include "common.h"
#include "task.h"
#include "math_Q.h"
#include "timer.h"


/**
* Sets external interrupt no 1 to fire on rising edge
*/
#define set_int1_to_rising()	{MCUCR |= (1 << ISC11) | (1 << ISC10);}

/**
* Sets external interrupt no 1 to fire on falling edge
*/
#define set_int1_to_falling()	{MCUCR &=  ~(1 << ISC10);}

/**
* Sets external interrupt no 0 to fire on rising edge
*/
#define set_int0_to_rising()	{MCUCR |= (1 << ISC01) | (1 << ISC00);}

/**
* Sets external interrupt no 0 to fire on falling edge
*/
#define set_int0_to_falling()	{MCUCR &=  ~(1 << ISC00);}

/**
* Eanbles external interrupts no 0 and 1
*/
#define eneable_external_int()	{GICR |= (1 << INT0) | (1 << INT1);}

/**
* Sends the value of the pwm register for the left motor
*/
#define send_left_m(x) {task_t m_info = {.data.command = MOTOR_L, .data.value = get_left_m()};add_task(&m_info);}

/**
* Sends the value of the pwm register for the right motor
*/
#define send_right_m(x){task_t m_info = {.data.command = MOTOR_R, .data.value = get_right_m()};add_task(&m_info);}

/**
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Don't use anywheres else than inside void do_cirecle(void) it contains local variables
* Creates a delay of specified ms and sets next state  after the timer expires
*/
#define circle_delay(time,next_state){if (do_once){do_once=false;tmr_start(&delay,time);}if (tmr_exp(&delay)){c_state = next_state;do_once=true;}}

/**
* pulses to rpm conversion
*/
//static const uint16_t rpm_speed[30]={15,29,44,58,73,87,102,116,131,145,160,174,189,203,218,233,247,262,276,291,305,320,334,349,363,378,392,407,422};

/**
* Motor specific variables
*/
volatile motor_t l_motor, r_motor;

/**
* All states present in the circle trick
*/
typedef enum
{
	PRE_DELAY,
	FORWARD_RADIUS,
	FIRST_DELAY,
	FIRST_CORNER,
	SECOND_DELAY,
	CIRCLE,
	THIRD_DELAY,
	SECOND_CORNR,
	FORTH_DELAY,
	REINIT,
} circle_state;

/**
* All states present in the circle trick
*/
typedef enum
{
	W_PRE_DELAY,
	W_FORWARD,
	W_DELAY_1,
	W_FIRST_CORNER,
	W_DELAY_2,
	W_WALL_1,
	W_DELAY_3,	
	W_SECOND_CORNR,
	W_DELAY_4,
	W_WALL_2,
	W_DELAY_5,
	W_THIRD_CORNR,
	W_DELAY_6,
	W_WALL_3,
	W_DELAY_7,
	W_FORTH_CORNR,
	W_DELAY_8,
	W_WALL_4,
	W_REINIT,
} wall_state;



/**
* \brief Initializes the external interrupts
*
* \param
*
* \return void
*/
void init_ext_int(void)
{
	set_int1_to_rising();
	set_int0_to_rising();
	eneable_external_int();
}

/**
* \brief Initializes the PWM counters
*
* \param
*
* \return void
*/
void init_pwm(void){
	//
	TCCR0 |= (1 << WGM00)|(1 << COM01)|(1 << WGM01)|(1 << CS00); //fast pwm ,64 prescalar, 8 bit
	TCCR2 |= (1 << WGM20)|(1 << WGM21)|(1 << COM21)|(1 << CS20); //fast pwm, 64 prescalar, 8 bit
	
	set_pin_as_output(B,PB3);
	set_pin_as_output(D,PD7);
	set_pin_as_output(C,PC0);
	set_pin_as_output(D,PD4);
	set_pin_as_output(D,PD5);
	set_pin_as_output(D,PD6);
}

#define check_corner(corner_t_value)\
{\
	if(motor->pulse_count>corner_t_value)\
	{\
		cli();\
		motor->rpm = 0;\
		motor->finished_corner=true;\
		sei();\
		motor->corner = C0;\
	}\
}
/**
* \brief  Checks if the motor finished the necessary corner
*
* \param motor Used to specify a reference to the motor that needs to be checked
*
* \return void
*/
void check_movement(volatile motor_t* motor)
{
	if (motor->corner!=C0)
	{
		switch (motor->corner)
		{
			case C90:
			{
				check_corner(C90);
			}
			break;
			case C45:
			{
				check_corner(C45)
			}
			break;
			case CIRCLE_RADIUS:
			{
				check_corner(CIRCLE_RADIUS)
			}
			break;
			case WALL_FORWARD:
			{
				check_corner(WALL_FORWARD)
			}
			break;
			case WALL_1:
			{
				check_corner(WALL_1)
			}
			break;
			case WALL_2:
			{
				check_corner(WALL_2)
			}
			break;
			case WALL_3:
			{
				check_corner(WALL_3)
			}
			break;
			case WALL_4:
			{
				check_corner(WALL_4)
			}
			break;
			default:
			{
				motor->rpm = 0;
				motor->corner = C0;
			}
			break;
		}
	}
}

/**
* \brief  Interface for making the robot turn in a specific direction, can be used to make the robot go in a direction for a specific distance
*
* \param rpm Used to specify the motor speed 50 rpms has the best output
* \param corner Used to specify the corner type
* \param d Used to specify Used to specyfy the direction
*
* \return void
*/
void set_movement(int16_t rpm, corner_t corner, direction_t d)
{
	switch (d)
	{
		case LEFT:
		{
			set_l_m_backward();
			set_r_m_forward();
		}
		break;
		case RIGHT:
		{
			set_l_m_forward();
			set_r_m_backward();
		}
		break;
		case FORWARD:
		{
			set_l_m_forward();
			set_r_m_forward();
		}
		break;
		case BACKWARD:
		{
			set_l_m_backward();
			set_r_m_backward();
		}
		break;
		default:
		{
			set_l_m_forward();
			set_r_m_forward();
		}
		break;
	}
	l_motor.rpm = rpm;
	r_motor.rpm = rpm;
	l_motor.corner = corner;//TODO maybe close interrupts for all the assignments
	r_motor.corner = corner;
	r_motor.pulse_count=0;
	l_motor.pulse_count=0;
}


/**
* \brief  Wrapper around set_corner function to be used with the usart interface
*
* \param task Used to specify a pointer to a specific received task.
*
* \return void
*/
void set_movement_task(task_t *task)
{
	static corner_t temp_corner= C0;
	switch (task->data.u8[2])
	{
		case 0:
			temp_corner = C0;
		break;
		case 1:
			temp_corner = C45;
		break;
		case 2:
			temp_corner = C90;
		break;
		case 3:
			temp_corner = C_FIND_BIT_MORE;
		break;
		default:
			temp_corner = C0;
		break;
	}
	set_movement(task->data.u8[3],temp_corner,task->data.u8[1]);
}

/**
* \brief  Handles all the motor checking
*
* \param
*
* \return void
*/
void motor_handler(void)
{
	static int16_t last_l_rpm=1,last_r_rpm=1;
	
	if(last_l_rpm!=l_motor.rpm)
	{
		if (l_motor.rpm>MAX_RPM)
		{
			l_motor.rpm = MAX_RPM;
		}
		else if (l_motor.rpm<1)
		{
			l_motor.rpm=0;
		}
		last_l_rpm = l_motor.rpm;
		l_motor.ref_pulses = l_motor.rpm / 16;
	}
	if (last_r_rpm!=r_motor.rpm)
	{
		if (r_motor.rpm>MAX_RPM)
		{
			r_motor.rpm = MAX_RPM;
		}
		else if (r_motor.rpm<1)
		{
			r_motor.rpm=0;
		}
		last_r_rpm = r_motor.rpm;
		r_motor.ref_pulses = r_motor.rpm / 16;
	}
	check_movement(&l_motor);
	check_movement(&r_motor);
	if (status.system.circle == true)//TODO: maybe it should be called from a state
	{
		do_cirecle();
	}
	if (status.system.wall == true)
	{
		
		do_wall();
	}
}


/**
* \brief  sets the motor rpm acording to the data.w[0] data.w[1] int values should be pased if negative robot will go backwords
*
* \param task Used to specify a pointer to a specific received task.
*
* \return void
*/
void set_rpm(task_t *task)
{
	u32_union temp;
	temp.dw = task->data.value;
	int16_t l = (int16_t)temp.w[1],r = (int16_t)temp.w[0];
	
	if (l>-1)
	{
		set_m_forward()
		l_motor.rpm = l;
		r_motor.rpm = r;
	}
	else
	{
		set_m_backward()
		l_motor.rpm = int16_abs_Q(l);
		r_motor.rpm = int16_abs_Q(r);
	}
	task_t motor2 = {.data.command = MOTOR_L, .data.value = l_motor.rpm};
	add_task(&motor2);
	task_t motor3 = {.data.command = MOTOR_R, .data.value = r_motor.rpm};
	add_task(&motor3);
}

	uint16_t circle_time = 4400;

	/**
	* \brief Tunes the time that takes to complete the circle
	*
	* \param task Used to specify a pointer to a specific received task. u8[3] represents a multiple of 100ms
	*
	* \return void
	*/
	void set_circle_time(task_t *task)
	{
		circle_time = task->data.u8[3]*100;
	}
	static uint16_t L=7, R=15;

	void set_corner_time(task_t *task)
	{
		R = task->data.u8[3];
		L = task->data.u8[2];
	}
	/**
	* \brief  makes a circle with a radius of ~50cm
	*
	* \param
	*
	* \return void
	*/
	void do_cirecle(void)
	{
		// 	TODO: create a state so nobody fucks with the robot
		static circle_state c_state = PRE_DELAY;
		static bool do_once = true;
		static timer_t delay;
		switch (c_state)
		{
			case PRE_DELAY:
			{
				circle_delay(250,FORWARD_RADIUS);
			}
			break;
			case FORWARD_RADIUS:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(180,CIRCLE_RADIUS,FORWARD);
				}
				if (movement_finished())
				{
					do_once = true;
					c_state=FIRST_DELAY;
				}
				
			}
			case FIRST_DELAY:
			{
				circle_delay(250,FIRST_CORNER);
			}
			break;
			case FIRST_CORNER:
			{
				if (do_once)
				{
					do_once = false;
					set_movement(180,C90,LEFT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=SECOND_DELAY;
				}
			}
			break;
			case SECOND_DELAY:
			{
				circle_delay(250,CIRCLE);
			}
			break;
			case CIRCLE:
			{
				if (do_once)
				{
					set_l_m_forward();
					set_r_m_forward();
					l_motor.rpm = L*16+1;
					r_motor.rpm = R*16+1;
					do_once=false;
					tmr_start(&delay,circle_time);
				}
				if (tmr_exp(&delay))
				{
					l_motor.rpm = 0;
					r_motor.rpm = 0;
					c_state = THIRD_DELAY;
					do_once=true;
				}
			}
			break;
			case THIRD_DELAY:
			{
				circle_delay(250,SECOND_CORNR);
			}
			break;
			case SECOND_CORNR:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(180,C90,RIGHT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=FORTH_DELAY;
				}
			}
			break;
			case FORTH_DELAY:
			{
				circle_delay(250,REINIT)
			}
			break;
			default:
			status.system.circle =false;
			c_state = PRE_DELAY;
			break;
		}
	}


	/**
	* \brief  makes a circle with a radius of ~50cm
	*
	* \param
	*
	* \return void
	*/
	void do_wall(void)
	{
		// 	TODO: create a state so nobody fucks with the robot
		static wall_state c_state = W_PRE_DELAY;
		static bool do_once = true;
		static timer_t delay;
		switch (c_state)
		{
			case W_PRE_DELAY:
			{
				circle_delay(3000,FORWARD_RADIUS);
			}
			break;
			case W_FORWARD:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(300,WALL_FORWARD,FORWARD);
				}
				if (movement_finished())
				{
					do_once = true;
					c_state=W_DELAY_1;
				}
				
			}
			case W_DELAY_1:
			{
				circle_delay(250,W_FIRST_CORNER);
			}
			break;
			case W_FIRST_CORNER:
			{
				if (do_once)
				{
					do_once = false;
					set_movement(180,C90,RIGHT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_2;
				}
			}
			break;
			case W_DELAY_2:
			{
				circle_delay(250,W_WALL_1);
			}
			break;
			case W_WALL_1:
			{				
				if (do_once)
				{
					do_once = false;
					set_movement(300,WALL_1,FORWARD);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_3;
				}
			}
			break;
			case W_DELAY_3:
			{
				circle_delay(250,W_SECOND_CORNR);
			}
			break;
			case W_SECOND_CORNR:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(180,C90,LEFT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_4;
				}
			}
			break;
			case W_DELAY_4:
			{
				circle_delay(250,W_WALL_2)
			}
			break;
			case W_WALL_2:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(300,WALL_2,FORWARD);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_5;
				}
			}
			break;
			case W_DELAY_5:
			{
				circle_delay(250,W_THIRD_CORNR)
			}
			break;
			case W_THIRD_CORNR:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(180,C90,LEFT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_6;
				}
			}
			break;
			case W_DELAY_6:
			{
				circle_delay(250,W_WALL_3)
			}
			break;
			case W_WALL_3:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(300,WALL_3,FORWARD);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_7;
				}
			}
			break;
			case W_DELAY_7:
			{
				circle_delay(250,W_FORTH_CORNR)
			}
			break;
			case W_FORTH_CORNR:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(180,C90,RIGHT);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_DELAY_8;
				}
			}
			break;
			case W_DELAY_8:
			{
				circle_delay(250,W_WALL_4)
			}
			break;
			case W_WALL_4:
			{
				if(do_once)
				{
					do_once=false;
					set_movement(300,WALL_4,FORWARD);
				}
				if (movement_finished())
				{
					do_once=true;
					c_state=W_REINIT;
				}
			}
			break;
			default:
			status.system.wall =false;
			c_state = PRE_DELAY;
			break;
		}
	}

	/**
	* \brief  Starts the circle trick
	*
	* \param task  Used to specify a pointer to a specific received task.
	*
	* \return void
	*/
	void start_circle(task_t *task)
	{
		do_cirecle();
		status.system.circle =true;
	}
	
	/**
	* \brief  Starts the circle trick
	*
	* \param task  Used to specify a pointer to a specific received task.
	*
	* \return void
	*/
	void start_wall(task_t *task)
	{
		do_wall();
		status.system.wall =true;
	}


	/**
	* \brief Initializes the motors
	*
	* \param
	*
	* \return void
	*/
	void motors_init(void)
	{
		init_ext_int();
		init_pwm();
		set_l_m_forward();
		set_r_m_forward();
		l_motor.ref_rpm=0;
		r_motor.ref_rpm=0;
		l_motor.rpm=1;
		r_motor.rpm=1;
		l_motor.break_count=1;
		r_motor.break_count=1;
	}