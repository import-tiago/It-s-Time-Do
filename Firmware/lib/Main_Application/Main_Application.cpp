#include "Main_Application.h"

struct Task_Parameters Task;
SystemStates Current_System_State = STARTING;

bool wifi_connected = false;

void trigger_output() {

	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);
	delay(300);
	pinMode(RELAY, INPUT_PULLUP);

	M5.Beep.beep();
	delay(50);
	M5.Beep.mute();
	delay(50);
}

void Start_Task() {

	trigger_output();

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
	uint8_t timeout = 10;

	Washing_Machine.Initializing = true;

	do {

		TFT_Clear();
		TFT_Wait_Task_Initialize_Screen(timeout);
		TFT_Print();

		Serial.println("Waiting LED power on...");
		delay(1000);

		if (!--timeout) {
			fail = true;
			Serial.print("task init fail!");
			JSON.set("/START", Washing_Machine.FAIL);
			Next_Task = Washing_Machine.FAIL;
			Send_Web_Push_Notification(Push_Notification.fail.TASK_FAIL);
		}

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

	const uint16_t SAMPLES = 256;
	float mean_voltage = 0;

	for (uint16_t i = 0; i < SAMPLES; i++)
		mean_voltage += analogReadMilliVolts(pin) / 1000.0F;

	mean_voltage /= SAMPLES;

	return (mean_voltage >= VOLTAGE_DROP_LED) ? true : false;
}