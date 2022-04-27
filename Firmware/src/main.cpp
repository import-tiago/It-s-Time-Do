#define FIRMWARE_VERSION "1.5"
/*
v1.0   - Initial release.
v1.1   - Bug fix in task duration calcs.
v1.2   - Update library 'Firebase-ESP32' from  v3.1.5 to v3.2.0.
v1.3   - Reading and writing data from/to server changed to JSON method.
       - More information besides 'task duration' added to the finished task log.
       - New data struct standard defined to store data in the real time database.
       - Print current task duration while waits the task finish.
       - Allows task duration monitor/log works from local or remote START trigger.
v1.4   - Enable Checks_OTA_Firmware_Update() on firmware start-up.
       - Enable Wi-Fi and Firebase connections status print on firmware start-up.
       - Checks if Firebase is ready before send JSON data to RTDB.
v1.4.1 - Bug fix: show working state at /START topic after remote trigger.
       - Bug fix: show free state at /START and /IoT_Device topics after task finished.
       - Send push notification when remote triggers occurs.
       - Calendar postion changed to fit firmware version text.
v1.4.2 - Bug fix: local trigger does not need trigger relay. Fixed.
v1.5   - Current firmware version sendind to RTDB.
*/

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

#define TASK_INIT 0
#define TASK_FINISH 1
#define REMOTE_TRIGGER 1
#define LOCAL_TRIGGER 1

struct Washing_Machine_Parameters {
    const String WORKING = "WORKING...";
    const String FREE = "FREE";

    bool starting = false;
    bool current_power_state = false;
    bool last_power_state = false;

    String washing_mode = "?";

    bool task_finished = false;

    String task_initial_time;
    String task_initial_date;

    String task_finished_time;
    String task_finished_date;

    String task_duration;

} Washing_Machine;

struct Task_Initialized_Parameters {
    String notification_icon_addr = "https://img.icons8.com/external-flaticons-lineal-color-flat-icons/344/external-time-gig-economy-flaticons-lineal-color-flat-icons-2.png";
    String notification_title = "TASK STARTED";
    String notification_body = "Your scheduled laundry has just started. You will be notified when it is finished.";
};

struct Task_Finished_Parameters {
    String notification_icon_addr = "https://img.icons8.com/external-tal-revivo-green-tal-revivo/344/external-verified-check-circle-for-approved-valid-content-basic-green-tal-revivo.png";
    String notification_title = "TASK FINISHED";
    String notification_body = "The washing task just finished!";
};

struct Notifications {
    struct Task_Initialized_Parameters init;
    struct Task_Finished_Parameters end;
    char Device_Tokens[10][200];
    uint8_t Number_Registered_Devices = 0;
} Push_Notification;

uint32_t Task_Initial_Timestamp = 0;
String Next_Task = "FREE";
String Start_Field;

bool local_start = false;

FirebaseJson JSON;
FirebaseJson JSON_Tokens;
FirebaseJsonData JSON_Field_Value;

void Its_Time_Do();
void Wait_Task_Finish();
String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp);
void Get_List_of_Web_Push_Notifications_Device_Tokens();

void Start_Washing_Machine(uint32_t init_timestamp, int8_t trigger_from);

void Check_and_Fix_Fields_in_RTDB() {

    Firebase.RTDB.getJSON(&fbdo, "/");
    fbdo.to<FirebaseJson>().get(JSON_Field_Value, "/START");

    if (!JSON_Field_Value.success) {
        JSON.add("START", Washing_Machine.FREE);
        Set_Firebase_JSON_at("/", JSON);
        JSON.clear();
    }
}

bool Get_Washing_Machine_Power_State(int pin) {

    const uint16_t SAMPLES = 512;
    uint32_t mean = 0;

    for (uint16_t i = 0; i < SAMPLES; i++)
        mean += analogRead(pin);

    mean /= SAMPLES;

    return (mean >= 220) ? true : false; // 220 in AD value  is ~0.7V (minimum drop voltage in a diode/LED)
}

void ISR_Display_Update() {

    OLED_Clear();

    if (!Washing_Machine.current_power_state)
        OLED_Build_Home_Screen(Next_Task, FIRMWARE_VERSION);
    else
        OLED_Build_Working_Screen(Task_Duration_Calc(Task_Initial_Timestamp, unix_time_in_seconds(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt())), FIRMWARE_VERSION);

    OLED_Print();
}

void ISR_Cloud_Communication() { //

    JSON.clear();

    Firebase.RTDB.getJSON(&fbdo, "/", &JSON);

    fbdo.to<FirebaseJson>().get(JSON_Field_Value, "/START");
    Next_Task = isValid_Time(JSON_Field_Value.to<String>()) != "-1" ? JSON_Field_Value.to<String>() : "FREE";

    Get_List_of_Web_Push_Notifications_Device_Tokens();

    JSON.set("/IoT_Device/Calendar", Current_Date(FULL));
    JSON.set("/IoT_Device/Clock", Current_Clock(WITHOUT_SECONDS));
    JSON.set("/IoT_Device/Schedule", (Washing_Machine.starting || Washing_Machine.current_power_state) ? Washing_Machine.WORKING : Next_Task);
    JSON.set("/IoT_Device/Firmware_Version", FIRMWARE_VERSION);

    JSON.set("/Washing_Machine/State", Washing_Machine.current_power_state ? "ON" : "OFF");

    if (isValid_Time(Next_Task) == "-1" || Washing_Machine.starting || local_start || Washing_Machine.current_power_state) {
        local_start = false;
        JSON.set("/START", (Washing_Machine.starting || Washing_Machine.current_power_state) ? Washing_Machine.WORKING : Washing_Machine.FREE);
    }

    if (Washing_Machine.task_finished) {
        static char path[100] = {0};
        Washing_Machine.task_finished = false;

        siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Finish", Washing_Machine.task_initial_date, Washing_Machine.task_initial_time.c_str());
        JSON.set(path, Washing_Machine.task_finished_time);

        siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Mode", Washing_Machine.task_initial_date, Washing_Machine.task_initial_time.c_str());
        JSON.set(path, Washing_Machine.washing_mode);

        siprintf(path, "/Washing_Machine/Last_Task/%s/%s/Duration", Washing_Machine.task_initial_date, Washing_Machine.task_initial_time.c_str());
        JSON.set(path, Washing_Machine.task_duration);

        JSON.set("/START", Washing_Machine.FREE);
        JSON.set("/IoT_Device/Schedule", Washing_Machine.FREE);
    }

    Set_Firebase_JSON_at("/", JSON);
}

void Send_Web_Push_Notification(int8_t type_message);

void ISR_Hardware_Inputs_Monitor() {
    Washing_Machine.current_power_state = Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED);
}

Ticker ISR_Display_Update_Controller(ISR_Display_Update, 100, 0, MILLIS);
Ticker ISR_Cloud_Communication_Controller(ISR_Cloud_Communication, 5000, 0, MILLIS);
Ticker ISR_Hardware_Inputs_Monitor_Controller(ISR_Hardware_Inputs_Monitor, 1000, 0, MILLIS);

void setup() {

    Board_Pins_Init();

    Flash_Memory_Init();

    Flash_Memory_Read_Variables();

    RTC_Init();

    OLED_Init();

    WiFi_Init();

    Firebase_Init();

    // Check_and_Fix_Fields_in_RTDB();

    Checks_OTA_Firmware_Update();

    OLED_Clear();

    OLED_Print_Loading_Screen();

    ISR_Display_Update_Controller.start();
    ISR_Cloud_Communication_Controller.start();
    ISR_Hardware_Inputs_Monitor_Controller.start();
}

void loop() {

    ISR_Display_Update_Controller.update();
    ISR_Cloud_Communication_Controller.update();
    ISR_Hardware_Inputs_Monitor_Controller.update();

    Its_Time_Do();

    Wait_Task_Finish();
}

void Its_Time_Do() {

    int hour = Next_Task.substring(0, Next_Task.indexOf(":")).toInt();
    int min = Next_Task.substring(Next_Task.indexOf(":") + 1).toInt();
    int sec = 0;

    int day = Current_Date(JUST_DAY).toInt();
    int month = Current_Date(JUST_MONTH).toInt();
    int year = Current_Date(JUST_YEAR).toInt();

    uint32_t current_timestamp = 0;

    current_timestamp = unix_time_in_seconds(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), sec, day, month, year);

    if (isValid_Time(Next_Task) != "-1") {

        uint32_t schedule_timestamp = 0;

        schedule_timestamp = unix_time_in_seconds(hour, min, sec, day, month, year);

        if (((current_timestamp >= schedule_timestamp) && (current_timestamp <= (schedule_timestamp + 120))) && !Washing_Machine.last_power_state) {
            Send_Web_Push_Notification(TASK_INIT);
            Start_Washing_Machine(current_timestamp, REMOTE_TRIGGER);
        }
    }

    if (!Washing_Machine.last_power_state && Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {
        Send_Web_Push_Notification(TASK_INIT);
        local_start = true;
        Start_Washing_Machine(current_timestamp, LOCAL_TRIGGER);
    }
}

void Start_Washing_Machine(uint32_t init_timestamp, int8_t trigger_from) {
    Washing_Machine.task_initial_time = Current_Clock(WITHOUT_SECONDS);
    Washing_Machine.task_initial_date = Current_Date(FULL);

    if (trigger_from == REMOTE_TRIGGER) {
        digitalWrite(RELAY, HIGH);
        delay(300);
        digitalWrite(RELAY, LOW);
    }

    while (!Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {
        Washing_Machine.starting = true;
        ISR_Display_Update_Controller.update();
        ISR_Cloud_Communication_Controller.update();
        ISR_Hardware_Inputs_Monitor_Controller.update();
        Serial.println("Waiting LED power on...");
        delay(500);
    }

    Washing_Machine.starting = false;
    Serial.print("task_initial_time: ");
    Serial.println(Washing_Machine.task_initial_time);

    Washing_Machine.last_power_state = true;
    Task_Initial_Timestamp = init_timestamp;
}

void Wait_Task_Finish() {

    if (Washing_Machine.last_power_state && !Get_Washing_Machine_Power_State(WASHING_MACHINE_POWER_LED)) {

        Washing_Machine.task_finished_time = Current_Clock(WITHOUT_SECONDS);
        Washing_Machine.task_finished_date = Current_Date(FULL);

        Serial.print("task_finished_time: ");
        Serial.println(Washing_Machine.task_finished_time);

        Washing_Machine.task_duration = Task_Duration_Calc(Task_Initial_Timestamp, unix_time_in_seconds(Current_Clock(JUST_HOUR).toInt(), Current_Clock(JUST_MIN).toInt(), 0, Current_Date(JUST_DAY).toInt(), Current_Date(JUST_MONTH).toInt(), Current_Date(JUST_YEAR).toInt()));

        Serial.print("Task_Duration: ");
        Serial.println(Washing_Machine.task_duration);

        Washing_Machine.last_power_state = false;

        Washing_Machine.task_finished = true;

        Send_Web_Push_Notification(TASK_FINISH);
    }
}

String Task_Duration_Calc(uint32_t init_timestamp, uint32_t end_timestamp) {

    uint32_t Task_Delta_Timestamp = end_timestamp - init_timestamp; // get delta in secs.
    Task_Delta_Timestamp /= 60;                                     // converts delta from secs to mins.
    int h = Task_Delta_Timestamp / 60;                              // converts delta from mins to hours and store only hours number and discarts (truncates) minutes.
    int m = ((Task_Delta_Timestamp / 60.0) - h) * 60;               // get only minutes from delta (in hours format) and convert this to minutes format.

    char Task_Duration[10];

    sprintf(Task_Duration, "%02dh%02dmin", h, m);

    return String(Task_Duration);
}

void Get_List_of_Web_Push_Notifications_Device_Tokens() {

    fbdo.to<FirebaseJson>().get(JSON_Field_Value, "/Notification_Tokens");

    JSON_Field_Value.get<FirebaseJson>(JSON_Tokens);

    size_t count = JSON_Tokens.iteratorBegin();

    Push_Notification.Number_Registered_Devices = count;

    for (size_t i = 0; i < count; i++) {
        FirebaseJson::IteratorValue value = JSON_Tokens.valueAt(i);
        sprintf(&Push_Notification.Device_Tokens[i][0], value.key.c_str());

        //   Serial.println(&Push_Notification.Device_Tokens[i][0]);
    }

    JSON_Tokens.iteratorEnd(); // required for free the used memory in iteration (node data collection)
}

void Send_Web_Push_Notification(int8_t type_message) {

    FCM_Legacy_HTTP_Message msg;

    FirebaseJsonArray arr;

    for (size_t i = 0; i < Push_Notification.Number_Registered_Devices; i++) {
        arr.add(&Push_Notification.Device_Tokens[i][0]);
        Serial.print("Push Notification Device Token: ");
        Serial.println(&Push_Notification.Device_Tokens[i][0]);
    }

    msg.targets.registration_ids = arr.raw();

    msg.options.time_to_live = "1000";
    msg.options.priority = "high";

    if (type_message == TASK_INIT) {
        msg.payloads.notification.title = Push_Notification.init.notification_title;
        msg.payloads.notification.body = Push_Notification.init.notification_body;
        msg.payloads.notification.icon = Push_Notification.init.notification_icon_addr;
    } else if (type_message == TASK_FINISH) {
        msg.payloads.notification.title = Push_Notification.end.notification_title;
        msg.payloads.notification.body = Push_Notification.end.notification_body;
        msg.payloads.notification.icon = Push_Notification.end.notification_icon_addr;
    }

    int8_t abort = 0;
    while (!Firebase.FCM.send(&fbdo, &msg)) {

        Serial.printf("push message send ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
        abort++;
        if (abort > 5)
            break;
    }
}