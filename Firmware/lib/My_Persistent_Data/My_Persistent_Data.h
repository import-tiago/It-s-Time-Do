#ifndef MY_PERSISTENT_DATA_H
#define MY_PERSISTENT_DATA_H

#include <Arduino.h>
#include <TridentTD_ESP32NVS.h>

static char Current_Firmware_Version[100] = {'\0'};


// PROTOTYPES
void Flash_Memory_Init();
void Flash_Memory_Read_Variables();


#endif // MY_PERSISTENT_DATA_H
