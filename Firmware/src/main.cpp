#define FIRMWARE_VERSION "1.12"
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
v1.7  	- 'Firebase-ESP32' library updated from v3.2.0 to v3.3.0.
		- Minor improvements in variables/functions names (more intuitive code reading).
		- Bug fix in Last_Task fild (data being erased after some time).
v1.8 	- Static variables changed to extern where appropriate.
v1.8.1 	- Now, Firebase_Get runs 'FirebaseReady' to ensure the connection in while loops.
		- 'Firebase-ESP32' library updated from v3.3.0 to v3.2.2 (author's downgrade)
v1.9 	- 'Firebase-ESP32' library updated from v3.2.2 to v3.3.2.
		- Ensures that when task is RUNNING the /IoT_Device/Schedule shows status as RUNNING too.
v1.10 	- 'Firebase-ESP32' library updated from v3.3.2 to v3.3.6.
		- 'RTClib' library updated from v2.0.2 to v2.0.3.
v1.11 	- 'Firebase-ESP32' library updated from v3.3.6 to v4.0.0.
		- 'download_buffer_size' increased.
v1.12 	- Fixed espressif32 platform at v3.5.0 because mbedtls (associated function) seems to have broken a bunch of libraries using it at new versions.
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
#include "Main_Application.h"

void System_States_Manager();

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

				Next_Task = Washing_Machine.FREE;

				Current_System_State = DISPLAY_UPDATE;

				break;
			}

		case DISPLAY_UPDATE: {

				OLED_Clear();

				if (Task.running)
					OLED_Build_Working_Screen(Task_Duration_Calc(Task.initial_timestamp, Get_Timestamp(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt())), FIRMWARE_VERSION);
				else
					OLED_Build_Home_Screen(Next_Task, FIRMWARE_VERSION);

				OLED_Print();

				Current_System_State = GET_CLOUD_JSON_DATA;
				break;
			}

		case GET_CLOUD_JSON_DATA: {

				JSON.clear();

				if (Get_Firebase_JSON_at("/", &JSON))
					Current_System_State = DESERIALIZE_JSON_DATA;

				break;
			}

		case DESERIALIZE_JSON_DATA: {

				Extract_List_of_Web_Push_Notifications_Device_Tokens();

				JSON_Deserialized.clear();

				JSON.get(JSON_Deserialized, "/START");

				JSON.remove("/START");

				if (JSON_Deserialized.success) {

					String hour = JSON_Deserialized.to<String>();

					if (isValid_Time(hour)) {

						Next_Task = hour;
						Current_System_State = REMOTE_TRIGGER_MONITOR;
					}
					else {

						if (!Task.running)
							Next_Task = Washing_Machine.FREE;

						Current_System_State = LOCAL_TRIGGER_MONITOR;
					}
				}
				else
					Current_System_State = GET_CLOUD_JSON_DATA;

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
					Remote_Start_Washing_Machine();
				else
					Current_System_State = LOCAL_TRIGGER_MONITOR;

				break;
			}

		case LOCAL_TRIGGER_MONITOR: {

				if (!Task.running && Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {

					Next_Task = Washing_Machine.WORKING;

					Task.running = true;

					Get_Task_Initialization_Parameters();

					Send_Web_Push_Notification(Push_Notification.init.TASK_INIT);
				}

				Current_System_State = TASK_STATUS_MONITOR;

				break;
			}

		case TASK_STATUS_MONITOR: {

				if (Task.running && !Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {

					Task.running = false;

					Task.new_report = true;

					Next_Task = Washing_Machine.FREE;

					Get_Task_Finalization_Parameters();

					Task.duration = Task_Duration_Calc(Task.initial_timestamp, Get_Timestamp(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt()));

					Send_Web_Push_Notification(Push_Notification.end.TASK_FINISH);
				}

				Current_System_State = SET_CLOUD_JSON;
				break;
			}

		case SET_CLOUD_JSON: {

				JSON.clear();

				while (!Get_Firebase_JSON_at("/", &JSON)) {
					;
				}

				JSON.remove("/START");

				do {
					JSON.set("/IoT_Device/Calendar", Current_Date(FULL));
					JSON.set("/IoT_Device/Clock", Current_Clock(WITHOUT_SECONDS));
					JSON.set("/IoT_Device/Schedule", Next_Task);
					JSON.set("/IoT_Device/Firmware_Version", FIRMWARE_VERSION);

					JSON.set("/Washing_Machine/State", Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED) ? "ON" : "OFF");

					if (Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED))
						JSON.set("/START", Washing_Machine.WORKING);

					if (Task.new_report) {

						Task.new_report = false;
						Task.running = false;

						char path[100] = { 0 };

						JSON.set("/START", Washing_Machine.FREE);

						siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Finish", Task.initial_date.c_str(), Task.initial_time.c_str());
						JSON.set(path, Task.finished_time);

						siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Mode", Task.initial_date.c_str(), Task.initial_time.c_str());
						JSON.set(path, Washing_Machine.washing_mode);

						siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Duration", Task.initial_date.c_str(), Task.initial_time.c_str());
						JSON.set(path, Task.duration);

					}

				} while (!Set_Firebase_JSON_at("/", &JSON));

				Current_System_State = DISPLAY_UPDATE;

				break;
			}

	}
}