#include "My_Persistent_Data.h"
#include <Arduino.h>
#include <TridentTD_ESP32NVS.h>

static char Current_Firmware_Version[100] = { 0 };

void Flash_Memory_Init() {
	NVS.begin();
}

void Flash_Memory_Read_Variables() {

	if (String((char*)NVS.getObject("FW")).length() > 1)
		strcpy((char*)Current_Firmware_Version, String((char*)NVS.getObject("FW")).c_str());

	if (!strlen((char*)Current_Firmware_Version))
		sprintf((char*)Current_Firmware_Version, "-1");
}