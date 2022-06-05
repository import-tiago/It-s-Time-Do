#include "Cloud.h"
#include "Firebase_Secrets.h"
#include "My_Persistent_Data.h"
#include "OLED.h"
#include <Arduino.h>
#include <TridentTD_ESP32NVS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h> //Provide the token generation process info.
#include <addons/RTDBHelper.h> //Provide the RTDB payload printing info and other helper functions.

FirebaseJson JSON;
FirebaseJson JSON_Tokens;
FirebaseJsonData JSON_Deserialized;
String Next_Task = Washing_Machine.FREE;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

struct Push_Notifications Push_Notification;
struct Washing_Machine_Parameters Washing_Machine;

bool isValid_Time(String time) {

	bool isValid = false;

	if (time.length() > 5)
		isValid = false;

	else if (time.indexOf(":") > 0) {

		int i = time.indexOf(':');
		int hour = time.substring(0, i).toInt();
		int min = time.substring(i + 1).toInt();

		if ((hour >= 0 && hour <= 23) && (min >= 0 && min <= 59))
			isValid = true;
	}
	else
		isValid = false;

	return isValid;
}

void fcsDownloadCallback(FCS_DownloadStatusInfo info) {

	if (info.status == fb_esp_fcs_download_status_init) {

		Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
	}
	else if (info.status == fb_esp_fcs_download_status_download) {
		Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
		OLED_OTA_Progress((int)info.progress);
	}
	else if (info.status == fb_esp_fcs_download_status_complete) {
		Serial.println("Update firmware completed.");
		Serial.println();
		Serial.println("Restarting...\n\n");
		delay(2000);
		ESP.restart();
	}
	else if (info.status == fb_esp_fcs_download_status_error) {
		Serial.printf("Download firmware failed, %s\n", info.errorMsg.c_str());
	}
}

void Download_New_Firmware_by_OTA() {

	if (!Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID, FIRMWARE_ADDRESS, fcsDownloadCallback))
		Serial.println(fbdo.errorReason());
}

void Checks_OTA_Firmware_Update() {

	Display_Check_OTA_Firmware_Update();

	Serial.println("Check_OTA_Firmware_Update");

	char New_Firmware_Version[100] = { '\0' };

	memset(New_Firmware_Version, '\0', sizeof(New_Firmware_Version));

	if (Firebase.ready()) {
		Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, FIRMWARE_ADDRESS) ? "ok" : fbdo.errorReason().c_str());

		if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {

			FileMetaInfo meta = fbdo.metaData();

			sprintf(New_Firmware_Version, String(meta.downloadTokens.c_str()).c_str());

			Flash_Memory_Read_Variables();

			Serial.print("LOCAL  Firmware Token: ");

			Serial.println(String((char*)NVS.getObject("FW")).c_str());

			Serial.print("SERVER Firmware Token: ");

			Serial.println(New_Firmware_Version);

			if (strcmp(String((char*)NVS.getObject("FW")).c_str(), New_Firmware_Version) != 0) {

				Serial.println("New firmware version available");

				NVS.setObject("FW", &New_Firmware_Version, sizeof(New_Firmware_Version));

				Download_New_Firmware_by_OTA();

			}
			else
				Serial.println("Firmware no needs update.");
		}
	}
}

void Firebase_Init() {
	Display_Firebase_Connecting();
	Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

	// Assign the api key (required)
	config.api_key = API_KEY;

	// Assign the user sign in credentials
	auth.user.email = USER_EMAIL;
	auth.user.password = USER_PASSWORD;

	/* Assign the RTDB URL (required) */
	config.database_url = DATABASE_URL;

	// Assign the callback function for the long running token generation task
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

	// Assign download buffer size in byte
	// Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
	// The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
	config.fcs.download_buffer_size = 1048576; // 1MB

	Firebase.begin(&config, &auth);

	Firebase.reconnectWiFi(true);

	Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
}

bool Set_Firebase_JSON_at(String Database_Path, FirebaseJson* json) {

	if (Firebase.ready()) {

		if (Firebase.RTDB.updateNode(&fbdo, Database_Path, json))
			return true;
		else
			Serial.println(fbdo.errorReason().c_str());
	}
	else
		return false;
}

bool Get_Firebase_JSON_at(String Database_Path, FirebaseJson* json) {

	if (Firebase.ready()) {

		if (Firebase.RTDB.getJSON(&fbdo, "/", json))
			return true;
		else
			Serial.println(fbdo.errorReason().c_str());
	}
	else
		return false;
}


void Extract_List_of_Web_Push_Notifications_Device_Tokens() {

	JSON_Deserialized.clear();

	JSON.get(JSON_Deserialized, "/Notification_Tokens");

	if (JSON_Deserialized.success) {
		JSON_Deserialized.get<FirebaseJson>(JSON_Tokens);

		size_t count = JSON_Tokens.iteratorBegin();

		Push_Notification.Number_Registered_Devices = count;

		for (size_t i = 0; i < count; i++) {
			FirebaseJson::IteratorValue value = JSON_Tokens.valueAt(i);
			sprintf(&Push_Notification.Device_Tokens[i][0], value.key.c_str());
			//Serial.println(value.key.c_str());
		}

		JSON_Tokens.iteratorEnd(); // required for free the used memory in iteration
	}
}

void Send_Web_Push_Notification(int8_t type_message) {

	FCM_Legacy_HTTP_Message msg;

	FirebaseJsonArray arr;

	for (size_t i = 0; i < Push_Notification.Number_Registered_Devices; i++)
		arr.add(&Push_Notification.Device_Tokens[i][0]);

	msg.targets.registration_ids = arr.raw();

	msg.options.time_to_live = "1000";
	msg.options.priority = "high";

	if (type_message == Push_Notification.init.TASK_INIT) {
		msg.payloads.notification.title = Push_Notification.init.notification_title;
		msg.payloads.notification.body = Push_Notification.init.notification_body;
		msg.payloads.notification.icon = Push_Notification.init.notification_icon_addr;
	}
	else if (type_message == Push_Notification.end.TASK_FINISH) {
		msg.payloads.notification.title = Push_Notification.end.notification_title;
		msg.payloads.notification.body = Push_Notification.end.notification_body;
		msg.payloads.notification.icon = Push_Notification.end.notification_icon_addr;
	}
	else if (type_message == Push_Notification.fail.TASK_FAIL) {
		msg.payloads.notification.title = Push_Notification.fail.notification_title;
		msg.payloads.notification.body = Push_Notification.fail.notification_body;
		msg.payloads.notification.icon = Push_Notification.fail.notification_icon_addr;
	}

	int8_t abort = 0;
	while (!Firebase.FCM.send(&fbdo, &msg)) {
		abort++;
		if (abort > 5)
			break;
	}
}