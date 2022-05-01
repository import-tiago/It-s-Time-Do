#define FIRMWARE_VERSION "1.6"
/*
v1.0    - Initial release.
v1.1    - Bug fix in task duration calcs.
v1.2    - Update library 'Firebase-ESP32' from  v3.1.5 to v3.2.0.
v1.3    - Reading and writing data from/to server changed to JSON method.
		- More information besides 'task duration' added to the finished task log.
		- New data struct standard defined to store data in the real time database.
		- Print current task duration while waits the task finish.
		- Allows task duration monitor/log works from local or remote START trigger.
v1.4    - Enable Checks_OTA_Firmware_Update() on firmware start-up.
		- Enable Wi-Fi and Firebase connections status print on firmware start-up.
		- Checks if Firebase is ready before send JSON data to RTDB.
v1.4.1  - Bug fix: show working state at /START topic after remote trigger.
		- Bug fix: show free state at /START and /IoT_Device topics after task finished.
		- Send push notification when remote triggers occurs.
		- Calendar postion changed to fit firmware version text.
v1.4.2  - Bug fix: local trigger does not need trigger relay. Fixed.
v1.5    - Current firmware version sendind to RTDB.
v1.6    - New firmware structure based on finite state machine, no more just ISR.
*/

/* Native libraries */
#include <Arduino.h>

/* Own libraries */
#include "Board_Pins.h"
#include "Cloud.h"
#include "DS3231.h"
#include "Firebase_Secrets.h"
#include "My_Persistent_Data.h"
#include "Network.h"
#include "OLED.h"
#include "WiFi_Secrets.h"

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

SystemStates Current_System_State = STARTING;

void System_States_Manager();
void System_State_Modify(SystemStates New_State);

void Start_Washing_Machine();
bool Get_Washing_Machine_Power_State(int pin);

void Get_Task_Initialization_Parameters();
void Get_Task_Finalization_Parameters();
bool Wait_Washing_Machine_Start();
String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp);

void setup() {

	Board_Pins_Init();

	Flash_Memory_Init();

	Flash_Memory_Read_Variables();

	RTC_Init();

	OLED_Init();

	WiFi_Init();

	Firebase_Init();

	Checks_OTA_Firmware_Update();

	OLED_Clear();

	OLED_Print_Loading_Screen();
}

void loop() {
	System_States_Manager();
}

void System_States_Manager() {

	switch (Current_System_State) {

		case STARTING: {

				System_State_Modify(DISPLAY_UPDATE);
				break;
			}

		case DISPLAY_UPDATE: {

				OLED_Clear();

				if (!Task.running)
					OLED_Build_Home_Screen(Next_Task, FIRMWARE_VERSION);
				else
					OLED_Build_Working_Screen(Task_Duration_Calc(Task.initial_timestamp, Get_Timestamp(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt())), FIRMWARE_VERSION);

				OLED_Print();

				System_State_Modify(GET_CLOUD_JSON_DATA);
				break;
			}

		case GET_CLOUD_JSON_DATA: {

				JSON.clear();

				Firebase.RTDB.getJSON(&fbdo, "/", &JSON);

				System_State_Modify(DESERIALIZE_JSON_DATA);

				break;
			}

		case DESERIALIZE_JSON_DATA: {

				Extract_List_of_Web_Push_Notifications_Device_Tokens();

				fbdo.to<FirebaseJson>().get(JSON_Result, "/START");
				JSON.remove("/START");
				if (isValid_Time(JSON_Result.to<String>())) {

					Next_Task = JSON_Result.to<String>();
					System_State_Modify(REMOTE_TRIGGER_MONITOR);
				}
				else
					System_State_Modify(LOCAL_TRIGGER_MONITOR);

				break;
			}

		case REMOTE_TRIGGER_MONITOR: {

				int hour = Next_Task.substring(0, Next_Task.indexOf(":")).toInt();
				int min = Next_Task.substring(Next_Task.indexOf(":") + 1).toInt();
				const int sec = 0;

				int day = Current_Date(JUST_DAY).toInt();
				int month = Current_Date(JUST_MONTH).toInt();
				int year = Current_Date(JUST_YEAR).toInt();

				uint32_t current_timestamp = Get_Current_Timestamp();
				uint32_t schedule_timestamp = Get_Timestamp(hour, min, sec, day, month, year);

				if (((current_timestamp >= schedule_timestamp) && (current_timestamp <= (schedule_timestamp + 120))) && !Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED))
					Start_Washing_Machine();
				else
					System_State_Modify(LOCAL_TRIGGER_MONITOR);

				break;
			}

		case LOCAL_TRIGGER_MONITOR: {

				if (!Task.running && Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {

					Serial.println("Task.running = true");

					Task.running = true;

					Next_Task = Washing_Machine.WORKING;

					Get_Task_Initialization_Parameters();

					Send_Web_Push_Notification(Push_Notification.init.TASK_INIT);
				}

				System_State_Modify(TASK_STATUS_MONITOR);

				break;
			}

		case TASK_STATUS_MONITOR: {

				if (Task.running && !Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {

					Serial.println("Task.running = false");

					Task.running = false;

					Task.done = true;

					Next_Task = Washing_Machine.FREE;

					Get_Task_Finalization_Parameters();

					Task.duration = Task_Duration_Calc(Task.initial_timestamp, Get_Timestamp(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt()));

					Send_Web_Push_Notification(Push_Notification.end.TASK_FINISH);
				}

				System_State_Modify(SET_CLOUD_JSON);
				break;
			}

		case SET_CLOUD_JSON: {

				JSON.set("/IoT_Device/Calendar", Current_Date(FULL));
				JSON.set("/IoT_Device/Clock", Current_Clock(WITHOUT_SECONDS));
				JSON.set("/IoT_Device/Schedule", Next_Task);
				JSON.set("/IoT_Device/Firmware_Version", FIRMWARE_VERSION);

				JSON.set("/Washing_Machine/State", Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED) ? "ON" : "OFF");


				if (Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED))
					JSON.set("/START", Washing_Machine.WORKING);

				if (Task.done) {

					Task.done = false;
					Task.running = false;

					static char path[100] = { 0 };

					JSON.set("/START", Washing_Machine.FREE);

					siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Finish", Task.initial_date, Task.initial_time.c_str());
					JSON.set(path, Task.finished_time);

					siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Mode", Task.initial_date, Task.initial_time.c_str());
					JSON.set(path, Washing_Machine.washing_mode);

					siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Duration", Task.initial_date, Task.initial_time.c_str());
					JSON.set(path, Task.duration);
				}

				Set_Firebase_JSON_at("/", JSON);

				System_State_Modify(DISPLAY_UPDATE);
				break;
			}

	}
}

void System_State_Modify(SystemStates New_State) {

	switch (New_State) {

		case STARTING:
			Current_System_State = STARTING;
			break;

		case GET_CLOUD_JSON_DATA:
			Current_System_State = GET_CLOUD_JSON_DATA;
			break;

		case DESERIALIZE_JSON_DATA:
			Current_System_State = DESERIALIZE_JSON_DATA;
			break;

		case REMOTE_TRIGGER_MONITOR:
			Current_System_State = REMOTE_TRIGGER_MONITOR;
			break;

		case LOCAL_TRIGGER_MONITOR:
			Current_System_State = LOCAL_TRIGGER_MONITOR;
			break;


		case TASK_STATUS_MONITOR:
			Current_System_State = TASK_STATUS_MONITOR;
			break;

		case SET_CLOUD_JSON:
			Current_System_State = SET_CLOUD_JSON;
			break;

		case DISPLAY_UPDATE:
			Current_System_State = DISPLAY_UPDATE;
			break;
	}
}

void Start_Washing_Machine() {

	digitalWrite(RELAY, HIGH);
	delay(300);
	digitalWrite(RELAY, LOW);

	if (!Wait_Washing_Machine_Start()) {

		Task.running = true;

		Next_Task = Washing_Machine.WORKING;

		Get_Task_Initialization_Parameters();

		Send_Web_Push_Notification(Push_Notification.init.TASK_INIT);

		System_State_Modify(TASK_STATUS_MONITOR);
	}
	else
		System_State_Modify(SET_CLOUD_JSON);

}

bool Wait_Washing_Machine_Start() {

	bool fail = false;
	uint8_t Task_Fail_Monitor = 0;
	const uint8_t TASK_FAIL_TIMEOUT = 20;

	Washing_Machine.starting = true;

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

	Washing_Machine.starting = false;

	return fail;
}

void Get_Task_Initialization_Parameters() {

	Task.initial_time = Current_Clock(WITHOUT_SECONDS);
	Task.initial_date = Current_Date(FULL);

	Serial.print("initial_time: ");
	Serial.println(Task.initial_time);

	Task.initial_timestamp = Get_Current_Timestamp();
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

	char Task_Duration[10];

	sprintf(Task_Duration, "%02dh%02dmin", h, m);

	Serial.print("Task_Duration: ");
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