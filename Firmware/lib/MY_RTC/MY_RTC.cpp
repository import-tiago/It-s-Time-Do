#include "MY_RTC.h"

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

String Current_Date(int format) {

	M5.Rtc.GetDate(&RTC_DateStruct);
	char data[11]; //dd/mm/yyyy

	if (format == JUST_DAY)
		sprintf(data, "%02d", RTC_DateStruct.Date);

	else if (format == JUST_MONTH)
		sprintf(data, "%02d", RTC_DateStruct.Month);

	else if (format == JUST_YEAR)
		sprintf(data, "%04d", RTC_DateStruct.Year);

	else if (format == FULL)
		sprintf(data, "%02d-%02d-%04d", RTC_DateStruct.Date, RTC_DateStruct.Month, RTC_DateStruct.Year);

	return &data[0];
}

String Current_Clock(int format) {

	M5.Rtc.GetTime(&RTC_TimeStruct);
	char data[9]; //hh/mm/ss

	if (format == FULL)
		sprintf(data, "%02d:%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);

	else if (format == JUST_HOUR)
		sprintf(data, "%02d", RTC_TimeStruct.Hours);

	else if (format == JUST_MIN)
		sprintf(data, "%02d", RTC_TimeStruct.Minutes);

	else
		sprintf(data, "%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);

	return &data[0];
}

uint32_t Get_Current_Timestamp() {

	M5.Rtc.GetTime(&RTC_TimeStruct);
	M5.Rtc.GetDate(&RTC_DateStruct);

	struct tm tm;

	tm.tm_hour = RTC_TimeStruct.Hours;
	tm.tm_min = RTC_TimeStruct.Minutes;
	tm.tm_sec = RTC_TimeStruct.Seconds;

	tm.tm_year = RTC_DateStruct.Year - 1900;
	tm.tm_mon = RTC_DateStruct.Month - 1;
	tm.tm_mday = RTC_DateStruct.Date;

	return mktime(&tm);
}

uint32_t Calc_Timestamp(uint16_t hour, uint16_t min, uint16_t sec, uint16_t day, uint16_t month, uint16_t year) {

	struct tm tm;

	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;

	return mktime(&tm);
}

void Set_RTC(char const* date, char const* time) {

	char s_month[5];
	int year;
	tmElements_t  t;
	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	sscanf(date, "%s %hhd %d", s_month, &t.Day, &year);
	sscanf(time, "%2hhd %*c %2hhd %*c %2hhd", &t.Hour, &t.Minute, &t.Second);

	// Find where is s_month in month_names. Deduce month value.
	t.Month = (strstr(month_names, s_month) - month_names) / 3 + 1;

	t.Year = year - 2000;

	RTC_TimeTypeDef TimeStruct;
	TimeStruct.Hours = t.Hour;
	TimeStruct.Minutes = t.Minute + 1;
	TimeStruct.Seconds = 30;
	M5.Rtc.SetTime(&TimeStruct);

	RTC_DateTypeDef DateStruct;
	DateStruct.Date = t.Day;
	DateStruct.Month = t.Month;
	DateStruct.Year = t.Year + 2000;
	M5.Rtc.SetDate(&DateStruct);
}