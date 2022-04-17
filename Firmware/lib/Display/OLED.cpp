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

void OLED_Print_Calendar(String calendar) {

    display.setTextSize(1);
    display.setCursor(32, 5);
    display.println(calendar);
}

void OLED_Print_Clock(String clock) {
    display.setTextSize(2);
    display.setCursor(15, 22);
    display.println(clock);
}

void OLED_Print_Schedule(String from_cloud) {
    display.setTextSize(1);
    display.setCursor(2, 50);
    display.print("NEXT TASK: ");
    display.print(from_cloud);
}
void OLED_Print_Loading_Screen() {
    display.setTextSize(2);
    display.setCursor(2, 25);
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

void OLED_Print_Firmware_Version(String Firmware_Version) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("v" + Firmware_Version);
}

void OLED_Build_Home_Screen(String _Schedule_Time, String Firmware_Version) {

    if (RTC_Status()) {
        OLED_Print_Firmware_Version(Firmware_Version);
        OLED_Print_Calendar(Current_Date(FULL));
        OLED_Print_Clock(Current_Clock(PRINT_SECONDS));
        OLED_Print_Schedule(_Schedule_Time);
    } else {
        display.setTextSize(2);
        display.setCursor(15, 25);
        display.print("RTC FAIL");
    }
}

String Schedule_Clock() {
    return ScheduleClock;
}

void OLED_Print() {
    display.display();
}