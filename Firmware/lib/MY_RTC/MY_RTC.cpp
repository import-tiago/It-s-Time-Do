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

void set_rtc_ic_from_internal_esp_rtc() {

	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		Serial.println("RTC ESP fail!");
		return;
	}

	RTC_TimeTypeDef TimeStruct;
	TimeStruct.Hours = timeinfo.tm_hour;
	TimeStruct.Minutes = timeinfo.tm_min;
	TimeStruct.Seconds = timeinfo.tm_sec;
	M5.Rtc.SetTime(&TimeStruct);

	RTC_DateTypeDef DateStruct;
	DateStruct.Date = timeinfo.tm_mday;
	DateStruct.WeekDay = timeinfo.tm_wday;
	DateStruct.Month = timeinfo.tm_mon;
	DateStruct.Year = timeinfo.tm_year;
	M5.Rtc.SetDate(&DateStruct);
}

void setTimezone(const char* timezone) {
	setenv("TZ", timezone, 1);
	tzset();
}

//https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
void set_internal_esp_rtc_from_ntp(const char* timezone) {
	struct tm timeinfo;
	configTime(0, 0, "pool.ntp.org");
	if (!getLocalTime(&timeinfo)) {
		Serial.println();
		Serial.print("NTP fail!");
		return;
	}
	setTimezone(timezone);
}

void print_local_time() {
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		Serial.println("RTC fail!");
		return;
	}
	Serial.println(&timeinfo, "%A, %d %B %Y - %H:%M:%S");
}