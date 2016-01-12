#include <stdio.h>
#include <stdlib.h>

#include "buttons.h"
#include "task.h"

void find_track_state_change() {
	state = STATE_FIND_TRACK;
	task_t system_state = {.data.command = STATE_COMMAND, .data.timestamp=0, .data.value=STATE_FIND_TRACK};
	add_task(&system_state);
}

void idle_state_change() {
	state = STATE_IDLE;
	task_t system_state = {.data.command = STATE_COMMAND, .data.timestamp=0, .data.value=STATE_IDLE};
	add_task(&system_state);
}