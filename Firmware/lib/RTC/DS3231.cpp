#include "DS3231.h"
#include "Board_Pins.h"
#include "RTClib.h"
#include <Arduino.h>

static RTC_DS3231 RTC;
static DateTime now;
static bool RTC_Init_Fail = false;

static char RTC_Buffer[11]; /* dd/mm/yyyy */

void RTC_Init() {
    if (!RTC.begin()) {
        while (1) {
            Serial.println("RTC Init Fail");
            RTC_Init_Fail = true;
        }
    }

    // if (RTC.lostPower())
    //     RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

bool RTC_Status() {
    return !RTC_Init_Fail;
}

String Current_Date(int format) {

    now = RTC.now();

    if (format == JUST_DAY)
        sprintf(RTC_Buffer, "%02d", now.day());

    else if (format == JUST_MONTH)
        sprintf(RTC_Buffer, "%02d", now.month());

    else if (format == JUST_YEAR)
        sprintf(RTC_Buffer, "%04d", now.year());

    else if (format == FULL)
        sprintf(RTC_Buffer, "%02d/%02d/%04d", now.day(), now.month(), now.year());

    return &RTC_Buffer[0];
}

String Current_Clock(int format) {

    now = RTC.now();
    if (format == PRINT_SECONDS)
        sprintf(RTC_Buffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    else if (format == JUST_HOUR)
        sprintf(RTC_Buffer, "%02d", now.hour());
    else if (format == JUST_MIN)
        sprintf(RTC_Buffer, "%02d", now.minute());
    else
        sprintf(RTC_Buffer, "%02d:%02d", now.hour(), now.minute());

    return &RTC_Buffer[0];
}

uint32_t
unix_time_in_seconds(uint8_t hrs, uint8_t min, uint8_t sec, uint8_t day, uint8_t mon, uint16_t year) {
    uint32_t ts = 0;

    //  Add up the seconds from all prev years, up until this year.
    uint8_t years = 0;
    uint8_t leap_years = 0;
    for (uint16_t y_k = EPOCH_YEAR; y_k < year; y_k++) {
        if (IS_LEAP_YEAR(y_k)) {
            leap_years++;
        } else {
            years++;
        }
    }
    ts += ((years * days_per_year[0]) + (leap_years * days_per_year[1])) * SEC_PER_DAY;

    //  Add up the seconds from all prev days this year, up until today.
    uint8_t year_index = (IS_LEAP_YEAR(year)) ? 1 : 0;
    for (uint8_t mo_k = 0; mo_k < (mon - 1); mo_k++) { //  days from previous months this year
        ts += days_per_month[year_index][mo_k] * SEC_PER_DAY;
    }
    ts += (day - 1) * SEC_PER_DAY; // days from this month

    //  Calculate seconds elapsed just today.
    ts += (uint32_t)hrs * SEC_PER_HOUR;
    ts += (uint32_t)min * SEC_PER_MIN;
    ts += (uint32_t)sec;

    return ts;
}