#ifndef DS3231_H
#define DS3231_H

#include <Arduino.h>

#define PRINT_SECONDS 1
#define WITHOUT_SECONDS 0

// PROTOTYPES
char *Current_Date();
char *Current_Clock(int format);
void RTC_Init();

#endif // DS3231_H
