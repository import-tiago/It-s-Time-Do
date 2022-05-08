#ifndef CLOUD_H
#define CLOUD_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

#define FIRMWARE_ADDRESS "bin/firmware.bin"

extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

extern FirebaseJson JSON;
extern FirebaseJson JSON_Tokens;
extern FirebaseJsonData JSON_Deserialized;
extern String Next_Task;

struct Washing_Machine_Parameters {
	const String WORKING = "WORKING...";
	const String FREE = "FREE";
	const String FAIL = "FAIL";

	bool Initializing = false;

	String washing_mode = "?";
};

extern struct Washing_Machine_Parameters Washing_Machine;

struct Notification_Task_Initialized_Parameters {
	String notification_icon_addr = "https://img.icons8.com/external-flaticons-lineal-color-flat-icons/344/external-time-gig-economy-flaticons-lineal-color-flat-icons-2.png";
	String notification_title = "TASK STARTED";
	String notification_body = "Your scheduled laundry has just started. You will be notified when it is finished.";
	const uint8_t TASK_INIT = 0;
};

struct Notification_Task_Finished_Parameters {
	String notification_icon_addr = "https://img.icons8.com/external-tal-revivo-green-tal-revivo/344/external-verified-check-circle-for-approved-valid-content-basic-green-tal-revivo.png";
	String notification_title = "TASK FINISHED";
	String notification_body = "The washing task just finished!";
	const uint8_t TASK_FINISH = 1;
};

struct Notification_Task_Fail_Parameters {
	String notification_icon_addr = "https://img.icons8.com/dusk/344/cancel.png";
	String notification_title = "TASK FAIL";
	String notification_body = "Unfortunately the task has not been started.";
	const uint8_t TASK_FAIL = 2;
};

struct Push_Notifications {
	struct Notification_Task_Initialized_Parameters init;
	struct Notification_Task_Finished_Parameters end;
	struct Notification_Task_Fail_Parameters fail;
	char Device_Tokens[10][200];
	uint8_t Number_Registered_Devices = 0;
};

extern struct Push_Notifications Push_Notification;

// PROTOTYPES
void fcsDownloadCallback(FCS_DownloadStatusInfo info);
void Download_New_Firmware_by_OTA();
void Checks_OTA_Firmware_Update();
void Firebase_Init();
bool isValid_Time(String from_cloud);
bool Set_Firebase_JSON_at(String Database_Path, FirebaseJson* json);
void Extract_List_of_Web_Push_Notifications_Device_Tokens();
void Send_Web_Push_Notification(int8_t type_message);

#endif // CLOUD_H
