/*
 * led.h
 *
 * Created: 24/04/2015 20:45:55
 *  Author: SavinA
 */ 


#ifndef LED_H_
#define LED_H_
#include "task.h"
typedef union
{
	struct
	{
		uint8_t sw0 : 1;
		uint8_t sw1 : 1;
		uint8_t sw2 : 1;
		uint8_t sw3 : 1;
		uint8_t sw4 : 1;
		uint8_t sw5 : 1;
		uint8_t sw6 : 1;
		uint8_t sw7 : 1;
	}switches;
	uint8_t array;
} led_t;


void led_init(void);
void get_line_error(void);
void sensor_eval(void);
void eval(void);
void send_sensor_values(void);
void start_line(task_t *task);
void set_l_Kp(task_t *task);
void set_l_Kd(task_t *task);
void set_l_Ki(task_t *task);
void set_pid(task_t *task);
void set_err4(task_t *task);
void set_err5(task_t *task);
void set_err6(task_t *task);
void set_err_p1(task_t *task);

extern volatile led_t led;

//TODO: delete either 0 or 7
#define read_switches(){led.array = (((0b00111111&PINA)<<1)|(PINB&0b00000001));}


#endif /* LED_H_ */