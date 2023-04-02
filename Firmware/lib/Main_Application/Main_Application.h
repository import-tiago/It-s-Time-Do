#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

#include <M5StickCPlus.h>
#include "MY_RTC.h"
#include "Board_Pins.h"
#include "Cloud.h"
#include "Display.h"

#define VOLTAGE_DROP_LED 2.00F

extern int8_t next_task_hour;
extern int8_t next_task_min;

extern bool wifi_connected;

typedef enum {

	STARTING = 0,

	SCHEDULED_TRIGGER_MONITOR,

	LOCAL_SCHEDULE_ADJUSTMENT,

	LOCAL_TRIGGER_MONITOR,

	TASK_STATUS_MONITOR,

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

void Start_Task();
bool Wait_Washing_Machine_Initialize();
void Get_Task_Initialization_Parameters();
void Get_Task_Finalization_Parameters();
String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp);
bool Get_Washing_Machine_Power_State(int pin);

#endif // MAIN_APPLICATION_H
