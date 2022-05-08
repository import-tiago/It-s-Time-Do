#include <Arduino.h>
#include "Main_Application.h"
#include "Board_Pins.h"
#include "Cloud.h"
#include "DS3231.h"

struct Task_Parameters Task;
SystemStates Current_System_State = STARTING;

void Remote_Start_Washing_Machine() {

	digitalWrite(RELAY, HIGH);
	delay(300);
	digitalWrite(RELAY, LOW);

	if (!Wait_Washing_Machine_Initialize()) {

		Next_Task = Washing_Machine.WORKING;

		Task.running = true;

		Get_Task_Initialization_Parameters();

		Send_Web_Push_Notification(Push_Notification.init.TASK_INIT);

		Current_System_State = TASK_STATUS_MONITOR;
	}
	else
		Current_System_State = SET_CLOUD_JSON;

}

bool Wait_Washing_Machine_Initialize() {

	bool fail = false;
	uint8_t Task_Fail_Monitor = 0;
	const uint8_t TASK_FAIL_TIMEOUT = 20;

	Washing_Machine.Initializing = true;

	do {
		Serial.println("Waiting LED power on...");
		delay(500);

		if (Task_Fail_Monitor >= TASK_FAIL_TIMEOUT) {
			fail = true;
			Serial.print("task init fail!");
			JSON.set("/START", Washing_Machine.FAIL);
			Next_Task = Washing_Machine.FAIL;
			Send_Web_Push_Notification(Push_Notification.fail.TASK_FAIL);
		}

		Task_Fail_Monitor++;
	} while (!Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED) && !fail);

	Washing_Machine.Initializing = false;

	return fail;
}

void Get_Task_Initialization_Parameters() {

	Task.initial_date = Current_Date(FULL);
	Task.initial_time = Current_Clock(WITHOUT_SECONDS);
	Task.initial_timestamp = Get_Current_Timestamp();

	Serial.print("initial_time: ");
	Serial.println(Task.initial_time);
}

void Get_Task_Finalization_Parameters() {

	Task.finished_time = Current_Clock(WITHOUT_SECONDS);
	Task.finished_date = Current_Date(FULL);

	Serial.print("finished_time: ");
	Serial.println(Task.finished_time);
}

String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp) {

	uint32_t Task_Delta_Timestamp = end_timestamp - init_timestamp; // get delta in secs.
	Task_Delta_Timestamp /= 60;         				  // converts delta from secs to mins.
	int h = Task_Delta_Timestamp / 60; 					  // converts delta from mins to hours and store only hours number and discarts (truncates) minutes.
	int m = ((Task_Delta_Timestamp / 60.0) - h) * 60;     // get only minutes from delta (in hours format) and convert this to minutes format.

	char Task_Duration[50];

	sprintf(Task_Duration, "%02dh%02dmin", h, m);

	Serial.print("Current Task_Duration: ");
	Serial.println(Task_Duration);

	return String(Task_Duration);
}

bool Get_Washing_Machine_Power_State(int pin) {

	const uint16_t SAMPLES = 255;
	uint32_t mean = 0;

	for (uint16_t i = 0; i < SAMPLES; i++)
		mean += analogRead(pin);

	mean /= SAMPLES;

	return (mean >= 220) ? true : false; // 220 in AD value  is ~0.7V (minimum drop voltage in a diode/LED)
}