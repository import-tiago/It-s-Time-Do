#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <M5StickCPlus.h>
#include "Free_Fonts.h"
#include "Main_Application.h"
#include "MY_RTC.h"

#include <WiFi.h>

extern TFT_eSprite tftSprite;

void init_tft_display();

void TFT_Build_Home_Screen(String _Schedule_Time, String Firmware_Version);
void TFT_Clear();
void TFT_Print();

void Display_Check_OTA_Firmware_Update();
void Display_WiFi_Connecting();
void Display_Firebase_Connecting();
void TFT_OTA_Progress(int status);

void TFT_Print_Calendar(String calendar);
void TFT_Print_Clock(String clock);
void TFT_Print_Schedule(String from_cloud);

void TFT_Print_Loading_Screen();
void Clear_Active_Tasks();
void TFT_Wait_Task_Initialize_Screen(uint8_t timeout);
void TFT_Build_Working_Screen(String current_duration, String Firmware_Version);


void TFT_Print_Current_Task_Duration(String value);

#endif // DISPLAY_H
