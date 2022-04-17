#ifndef DS3231_H
#define DS3231_H

#include <Arduino.h>

#include <stdint.h>

#define SEC_PER_MIN 60
#define SEC_PER_HOUR 3600
#define SEC_PER_DAY 86400
#define MOS_PER_YEAR 12
#define EPOCH_YEAR 1970
#define IS_LEAP_YEAR(year) ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))

static int days_per_month[2][MOS_PER_YEAR] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static int days_per_year[2] = {
    365, 366};

uint32_t
unix_time_in_seconds(uint8_t hrs, uint8_t min, uint8_t sec, uint8_t day, uint8_t mon, uint16_t year);

#define PRINT_SECONDS 1
#define WITHOUT_SECONDS 0

#define JUST_DAY 0
#define JUST_MONTH 1
#define JUST_YEAR 2
#define FULL 3

#define JUST_HOUR 1
#define JUST_MIN 2

// PROTOTYPES
String Current_Date(int format);
String Current_Clock(int format);
void RTC_Init();
bool RTC_Status();

#endif // DS3231_H
