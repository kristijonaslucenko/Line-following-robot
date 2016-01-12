/*
 * control_logic.h
 *
 * Created: 5/16/2015 3:38:45 PM
 *  Author: Adam Orosz, s134615
 */ 


#ifndef CONTROL_LOGIC_H_
#define CONTROL_LOGIC_H_
#include "timer.h"

typedef struct  
{
	bool not_first_run;
	bool exp;
} state_data_t;

extern bool state_changed;
extern volatile state_data_t state_find_track_data;
extern volatile state_data_t state_find_track_go_a_bit_more_control_data;
extern volatile state_data_t state_find_track_turn_left_control_data;
extern volatile state_data_t state_wait_before_corner_data;
extern volatile state_data_t state_approach_corner_data;
extern volatile state_data_t state_turn_after_found_corner_data;
extern volatile state_data_t state_go_ahead_after_turn_data;
extern volatile state_data_t state_go_a_bit_more_data;
extern volatile state_data_t state_second_left_turn_data;
extern volatile state_data_t state_second_go_ahead_data;
extern volatile state_data_t state_second_go_a_bit_more_data;
extern volatile state_data_t state_third_left_turn_data;
extern volatile state_data_t state_third_go_ahead_data;
extern volatile state_data_t state_right_turn_data;
extern volatile state_data_t state_last_go_ahead_data;
extern volatile state_data_t state_finish_data;

extern timer_t state_timer;


void state_idle_control_logic();
void state_find_track_control_logic();
void state_find_track_go_a_bit_more_control_logic();
void state_find_track_turn_left_control_logic();
void state_follow_track_1_control_logic();
void state_y_intersection_control_logic();
void state_follow_track_2_control_logic();
void state_wait_before_corner_logic();
void state_approach_corner_logic();
void state_turn_after_found_corner_logic();
void state_go_ahead_after_turn_logic();
void state_go_a_bit_more_logic();
void state_second_left_turn_logic();
void state_second_go_ahead_logic();
void state_second_go_a_bit_more_logic();
void state_third_left_turn_logic();
void state_third_go_ahead_logic();
void state_right_turn_logic();
void state_last_go_ahead_logic();
void state_finish_logic();
void state_take_over_control_logic();
extern volatile void (*control)();


#endif /* CONTROL_LOGIC_H_ */