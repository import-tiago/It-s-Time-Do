#ifndef CLOUD_H
#define CLOUD_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

#define FIRMWARE_ADDRESS "/firmware.bin"
static String ScheduleClock;
static String Last_Start_Time;

// Define Firebase Data object
static FirebaseData fbdo;

static FirebaseAuth auth;
static FirebaseConfig config;

// PROTOTYPES
bool Get_Firebase_Bool_from(char *Database_Path);
void Set_Firebase_Bool_at(char *Database_Path, bool data);
String Get_Firebase_String_from(char *Database_Path);
void Set_Firebase_String_at(char *Database_Path, char *data);
void fcsDownloadCallback(FCS_DownloadStatusInfo info);
void Download_New_Firmware_by_OTA();
void Checks_OTA_Firmware_Update();
void Firebase_Init();
String isValid_Time(String from_cloud);


#endif // CLOUD_H
