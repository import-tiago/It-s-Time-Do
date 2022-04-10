#include "Firebase_Secrets.h"
#include "WiFi_Secrets.h"
#include "Board_Pins.h"
#include "Cloud.h"
#include "My_Persistent_Data.h"
#include "Network.h"
#include "OLED.h"
#include <Arduino.h>

void setup() {

    Board_Pins_Init();

    Flash_Memory_Init();

    Flash_Memory_Read_Variables();

    RTC_Init();

    OLED_Init();

    WiFi_Init();

    Firebase_Init();

    Checks_OTA_Firmware_Update();

    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);

    OLED_Clear();
}

void loop() {
    OLED_Print_Home_Screen();
}