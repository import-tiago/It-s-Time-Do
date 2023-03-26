#ifndef RTC_H
#define RTC_H

#include <M5StickCPlus.h>
#include <TimeLib.h>

#define PRINT_SECONDS 1
#define WITHOUT_SECONDS 0

#define JUST_DAY 0
#define JUST_MONTH 1
#define JUST_YEAR 2
#define FULL 3

#define JUST_HOUR 1
#define JUST_MIN 2


extern RTC_TimeTypeDef RTC_TimeStruct;
extern RTC_DateTypeDef RTC_DateStruct;

// PROTOTYPES
String Current_Date(int format);
String Current_Clock(int format);
uint32_t Get_Current_Timestamp();
uint32_t Calc_Timestamp(uint16_t hour, uint16_t min, uint16_t sec, uint16_t day, uint16_t month, uint16_t year);
void Set_RTC(char const* date, char const* time);
//uint32_t Get_Timestamp(uint8_t hrs, uint8_t min, uint8_t sec, uint8_t day, uint8_t mon, uint16_t year);

#endif //RTC_H