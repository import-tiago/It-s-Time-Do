#include "OLED.h"
#include "Cloud.h"
#include "DS3231.h"
#include "WiFi_Secrets.h"
#include <Arduino.h>

#include <Firebase_ESP_Client.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#define PRINT_SECONDS 1
#define WITHOUT_SECONDS 0

static Adafruit_SSD1306 display(128, 64, &Wire, -1);
static char dateBuffer[50];



void OLED_Init() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED Init Fail"));
        while (1) {
            ;
        }
    }
    display.setTextColor(WHITE);
}

void Display_Check_OTA_Firmware_Update() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(15, 5);
    display.print("CHECKING");
    display.setCursor(15, 25);
    display.print("FIRMWARE");
    display.setCursor(25, 45);
    display.print("UPDATE");
    display.display();
}

void OLED_Print_Calendar(char *calendar) {

    display.setTextSize(1);
    display.setCursor(32, 5);
    display.println(calendar);
}

void OLED_Print_Clock(char *clock) {
    display.setTextSize(1);
    display.setCursor(37, 15);
    display.println(clock);
}

void OLED_Print_Schedule(String from_cloud) {
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print(from_cloud);
}
void OLED_Print_Loading_Screen() {
    display.setTextSize(2);
    display.setCursor(2, 30);
    display.print("LOADING...");
    display.display();
}
void OLED_Clear() {
    display.clearDisplay();
}

void Display_WiFi_Connecting() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5, 5);
    display.print("Wi-Fi Connecting...");
    display.setCursor(5, 20);
    display.print(WIFI_SSID);
    display.display();
}

void Display_Firebase_Connecting() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5, 5);
    display.print("Firebase");
    display.setCursor(5, 20);
    display.print("Connecting...");
    display.display();
}

void OLED_OTA_Progress(int status) {
    display.clearDisplay();
    display.setTextColor(WHITE);

    display.setTextSize(2);
    display.setCursor(10, 5);
    display.print("UPDATING");

    display.setTextSize(4);
    display.setCursor(10, 30);
    display.print(String(status) + "%");
    display.display();
}

void OLED_Build_Home_Screen(String _Schedule_Time) {

     OLED_Print_Calendar(Current_Date());
     OLED_Print_Clock(Current_Clock(PRINT_SECONDS));
     OLED_Print_Schedule(_Schedule_Time);

    /*

    static int lastMinute = 0;
    static int lastSecond = 0;


    if (now.second() > lastSecond || !now.second()) {
        lastSecond = now.second();

        // CALENDAR
        memset(dateBuffer, '\0', sizeof(dateBuffer));
        sprintf(dateBuffer, "%02d/%02d/%04d", now.day(), now.month(), now.year());
        OLED_Print_Calendar(dateBuffer);



        // CL0OCK
        memset(dateBuffer, '\0', sizeof(dateBuffer));
        sprintf(dateBuffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        OLED_Print_Clock(dateBuffer);
        memset(dateBuffer, '\0', sizeof(dateBuffer));
        sprintf(dateBuffer, "%02d:%02d", now.hour(), now.minute());
        if (now.minute() > lastMinute || !now.minute()) {
            lastMinute = now.minute();
            Set_Firebase_String_at("/Device Clock", dateBuffer);
        }

        // GET SCHEDULE
        Last_Start_Time = isValid_Time(Get_Firebase_String_from("/START"));

        if (Last_Start_Time != "-1")
            ScheduleClock = Last_Start_Time;

        Set_Firebase_String_at("/TASK", (char *)ScheduleClock.c_str());

        sprintf(dateBuffer, "START: %s", ScheduleClock);
        OLED_Print_Schedule(dateBuffer);
    }
    */
}

String Schedule_Clock() {
    return ScheduleClock;
}

void Clear_Active_Tasks() {
    /*
        char end_data[100];
        Serial.print("ScheduleClock: ");
        Serial.println(ScheduleClock);

        sprintf(end_data, "START: %s, FINISH: %02d:%02d, in %02d/%02d/%04d", ScheduleClock, now.hour(), now.minute(), now.day(), now.month(), now.year());

        Set_Firebase_String_at("/START", "-1");
        Set_Firebase_String_at("/TASK", "-1");
        Set_Firebase_String_at("/FINISH", end_data);
        ScheduleClock = "-1";
       */
}

void OLED_Print() {
    display.display();
}