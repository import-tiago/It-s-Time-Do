#include "OLED.h"
#include "WiFi_Secrets.h"
#include <Arduino.h>

#include "Cloud.h"
#include <Firebase_ESP_Client.h>
#include "RTClib.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static RTC_DS3231 RTC;
 static DateTime now;



void RTC_Init(){
        if (!RTC.begin()) {
        Serial.println("RTC Init Fail");
        while (1) {
            ;
        }
    }
}

void OLED_Init() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED Init Fail"));
        while (1) {
            ;
        }
    }
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
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(32, 5);
    display.println(calendar);
}

void OLED_Print_Clock(char *clock) {
    display.setTextSize(1);
    display.setCursor(35, 15);
    display.println(clock);
}

void OLED_Print_Schedule(char *from_cloud) {
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print(from_cloud);
    display.display();
}

void OLED_Clear() {
    display.clearDisplay();
    display.display();
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

void OLED_Print_Home_Screen() {

    static char dateBuffer[50];
    static int lastMinute = 0;
    static int lastSecond = 0;
    now = RTC.now();


        if (now.second() > lastSecond || !now.second()) {
            lastSecond = now.second();

            // CALENDAR
            memset(dateBuffer, '\0', sizeof(dateBuffer));
            sprintf(dateBuffer, "%02d/%02d/%04d", now.day(), now.month(), now.year());
            OLED_Print_Calendar(dateBuffer);
            Set_Firebase_String_at("/Device Calendar", dateBuffer);

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

            sprintf(dateBuffer, "START: %s", Get_Firebase_String_from("/START"));
            OLED_Print_Schedule(dateBuffer);
        }

}