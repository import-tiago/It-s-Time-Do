#define FIRMWARE_VERSION "1.1"

/* Native libraries */
#include <Arduino.h>

/* External libraries */
#include <Ticker.h>

/* Own libraries */
#include "Board_Pins.h"
#include "Cloud.h"
#include "DS3231.h"
#include "Firebase_Secrets.h"
#include "My_Persistent_Data.h"
#include "Network.h"
#include "OLED.h"
#include "WiFi_Secrets.h"

uint32_t Task_Initial_Timestamp = 0;
bool Wash_Machine_Power = false;
String Schedule_on_Server = "";

void Its_Time_Do();
void Wait_Task_Finish_and_Calc_Duration();

bool Wash_Machine_Power_State(int pin) {
    bool state = false;
    const uint32_t SAMPLES = 100;

    uint32_t mean = 0;

    for (int i = 0; i < SAMPLES; i++)
        mean += (uint32_t)analogRead(pin);

    mean /= SAMPLES;

    if (mean >= 220) // 250AD = 0,7V
        state = true;

    else
        state = false;

    return state;
}

void ISR_Display_Update() {
    OLED_Clear();
    OLED_Build_Home_Screen(Schedule_on_Server != "" ? Schedule_on_Server : "FREE", FIRMWARE_VERSION);
    OLED_Print();
}

void ISR_Server_Monitor() {
    Schedule_on_Server = isValid_Time(Get_Firebase_String_from("/START"));
}

void ISR_Server_Update() {
    Set_Firebase_String_at("/Device/Calendar", Current_Date(FULL));
    Set_Firebase_String_at("/Device/Clock", Current_Clock(WITHOUT_SECONDS));
    Set_Firebase_String_at("/Device/Schedule", (Schedule_on_Server != "" || Schedule_on_Server == "WORKING...") ? Schedule_on_Server : "FREE");
    Set_Firebase_Bool_at("/LED", Wash_Machine_Power_State(WASH_MACHINE_POWER_LED));
}

Ticker ISR_Display_Update_Controller(ISR_Display_Update, 100, 0, MILLIS);
Ticker ISR_Server_Monitor_Controller(ISR_Server_Monitor, 100, 0, MILLIS);
Ticker ISR_Server_Update_Controller(ISR_Server_Update, 100, 0, MILLIS);

void setup() {

    Board_Pins_Init();

    Flash_Memory_Init();

    Flash_Memory_Read_Variables();

    RTC_Init();

    OLED_Init();

    WiFi_Init();

    Firebase_Init();

    Checks_OTA_Firmware_Update();

    OLED_Clear();

    OLED_Print_Loading_Screen();

    ISR_Display_Update_Controller.start();
    ISR_Server_Update_Controller.start();
    ISR_Server_Monitor_Controller.start();
}

void loop() {

    ISR_Display_Update_Controller.update();
    ISR_Server_Update_Controller.update();
    ISR_Server_Monitor_Controller.update();

    Its_Time_Do();

    Wait_Task_Finish_and_Calc_Duration();
}

void Its_Time_Do() {

    if (isValid_Time(Schedule_on_Server) && (Schedule_on_Server != "FREE") && (Schedule_on_Server != "WORKING...")) {

        int hour = Schedule_on_Server.substring(0, Schedule_on_Server.indexOf(":")).toInt();
        int min = Schedule_on_Server.substring(Schedule_on_Server.indexOf(":") + 1).toInt();
        int sec = 0;

        int day = Current_Date(JUST_DAY).toInt();
        int month = Current_Date(JUST_MONTH).toInt();
        int year = Current_Date(JUST_YEAR).toInt();

        uint32_t schedule_timestamp = 0, current_timestamp = 0;

        schedule_timestamp = unix_time_in_seconds(hour, min, sec, day, month, year);
        current_timestamp = unix_time_in_seconds(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), sec, day, month, year);

        if (((current_timestamp >= schedule_timestamp) && (current_timestamp <= (schedule_timestamp + 120))) && !Wash_Machine_Power) {

            digitalWrite(RELAY, HIGH);
            delay(300);
            digitalWrite(RELAY, LOW);

            while (!Wash_Machine_Power_State(WASH_MACHINE_POWER_LED)) {
                ;
            }

            Wash_Machine_Power = true;
            Task_Initial_Timestamp = current_timestamp;

            while (!Set_Firebase_String_at("/START", "WORKING...")) {
                ;
            }

            Schedule_on_Server = "WORKING...";

            while (!Set_Firebase_String_at("/Device/Schedule", "WORKING...")) {
                ;
            }
        }
    }
}

void Wait_Task_Finish_and_Calc_Duration() {

    if (Wash_Machine_Power && !Wash_Machine_Power_State(WASH_MACHINE_POWER_LED)) {

        uint32_t Task_Delta_Timestamp = 0, Task_Finished_Timestamp = 0;

        Task_Finished_Timestamp = unix_time_in_seconds(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt());

        Task_Delta_Timestamp = Task_Finished_Timestamp - Task_Initial_Timestamp; // get delta in secs

        Task_Delta_Timestamp /= 60; // converts delta from secs to mins

        int h = Task_Delta_Timestamp / 60; // converts delta from mins to hours and store only hours number and discarts (truncates) minutes

        int m = ((Task_Delta_Timestamp / 60.0) - h) * 60; // get only minutes from delta (in hours format) and convert this to minutes format

        char Task_Duration[20];

        sprintf(Task_Duration, "%02dh%02dmin", h, m);

        Set_Firebase_String_at("/LAST_TASK/" + String(Task_Finished_Timestamp), Task_Duration);

        Wash_Machine_Power = false;

        while (!Set_Firebase_String_at("/START", "FREE")) {
            ;
        }
    }
}