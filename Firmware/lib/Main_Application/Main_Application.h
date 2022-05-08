#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

#include <Arduino.h>

typedef enum {

	STARTING = 0,

	GET_CLOUD_JSON_DATA,

	DESERIALIZE_JSON_DATA,

	REMOTE_TRIGGER_MONITOR,

	LOCAL_TRIGGER_MONITOR,

	TASK_STATUS_MONITOR,

	SET_CLOUD_JSON,

	DISPLAY_UPDATE

} SystemStates;

extern SystemStates Current_System_State;

struct Task_Parameters {
	bool running;
	bool new_report;

	String initial_time;
	String initial_date;
	uint32_t initial_timestamp;

	String finished_time;
	String finished_date;

	String duration;
};

extern struct Task_Parameters Task;

void Remote_Start_Washing_Machine();
bool Wait_Washing_Machine_Initialize();
void Get_Task_Initialization_Parameters();
void Get_Task_Finalization_Parameters();
String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp);
bool Get_Washing_Machine_Power_State(int pin);

#endif // MAIN_APPLICATION_H
