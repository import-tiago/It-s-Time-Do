#include "DS3231.h"
#include "RTClib.h"
#include <Arduino.h>

static RTC_DS3231 RTC;
static DateTime now;

static char RTC_Buffer[11]; /* dd/mm/yyyy */

void RTC_Init() {
    if (!RTC.begin()) {
        Serial.println("RTC Init Fail");
        while (1) {
            ;
        }
    }
}

char *Current_Date() {

    now = RTC.now();

    sprintf(RTC_Buffer, "%02d/%02d/%04d", now.day(), now.month(), now.year());

    return &RTC_Buffer[0];
}

char *Current_Clock(int format) {

    now = RTC.now();
    if (format == PRINT_SECONDS)
        sprintf(RTC_Buffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    else
        sprintf(RTC_Buffer, "%02d:%02d", now.hour(), now.minute());
    return &RTC_Buffer[0];
}