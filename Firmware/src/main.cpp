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

#define LED_NETWORK_STATUS 2

unsigned long MOSFET_Trigger_Time[NUMBER_OF_BUTTONS][3];

#define MOSFET_1_STATE (MOSFET_Trigger_Time[0][0])
#define MOSFET_2_STATE (MOSFET_Trigger_Time[1][0])
#define MOSFET_3_STATE (MOSFET_Trigger_Time[2][0])
#define MOSFET_4_STATE (MOSFET_Trigger_Time[3][0])
#define MOSFET_5_STATE (MOSFET_Trigger_Time[4][0])

#define MOSFET_1_TRIGGER_INIT_TIME (MOSFET_Trigger_Time[0][1])
#define MOSFET_2_TRIGGER_INIT_TIME (MOSFET_Trigger_Time[1][1])
#define MOSFET_3_TRIGGER_INIT_TIME (MOSFET_Trigger_Time[2][1])
#define MOSFET_4_TRIGGER_INIT_TIME (MOSFET_Trigger_Time[3][1])
#define MOSFET_5_TRIGGER_INIT_TIME (MOSFET_Trigger_Time[4][1])

#define MOSFET_DISABLED 0
#define MOSFET_ENABLED 1

String Schedule_Time = "";

void Remote_Buttons_Monitor();
void Remote_Schedule_Monitor();

// 21 22 I2C

struct Wash_Machine_Panel {
    bool LED_Ciclo_Iniciado = false;
    bool LED_Turbo_Agitacao = false;
    bool LED_Turbo_Secagem = false;
    bool LED_Reutilizar_Agua = false;
    bool LED_Centrifugando = false;
    bool LED_Enxaguando = false;
    bool LED_Agitacao_Curta = false;
    bool LED_Agitacao_Normal = false;
    bool LED_Agitacao_Longa = false;
};
struct Wash_Machine_Panel Wash_Machine;

void ISR_GPIOs_Read() {
    Wash_Machine.LED_Ciclo_Iniciado = digitalRead(LED_CICLO_INICIADO_PIN);
    Wash_Machine.LED_Turbo_Agitacao = digitalRead(LED_TURBO_AGITACAO_PIN);
    Wash_Machine.LED_Turbo_Secagem = digitalRead(LED_TURBO_SECAGEM_PIN);
    Wash_Machine.LED_Reutilizar_Agua = digitalRead(LED_REUTILIZAR_AGUA_PIN);
    Wash_Machine.LED_Centrifugando = digitalRead(LED_CENTRIFUGANDO_PIN);
    Wash_Machine.LED_Enxaguando = digitalRead(LED_ENXAGUANDO_PIN);
    Wash_Machine.LED_Agitacao_Curta = digitalRead(LED_AGITACAO_CURTA_PIN);
    Wash_Machine.LED_Agitacao_Normal = digitalRead(LED_AGITACAO_NORMAL_PIN);
    Wash_Machine.LED_Agitacao_Longa = digitalRead(LED_AGITACAO_LONGA_PIN);
}

void ISR_Display_Update() {

    digitalWrite(LED, !digitalRead(LED));

    OLED_Clear();
    OLED_Build_Home_Screen(Schedule_Time != "" ? Schedule_Time : "NO SCHEDULE");
    OLED_Print();

    Serial.println(Schedule_Time);
}

void ISR_Server_Monitor() {

    Remote_Buttons_Monitor();

    Remote_Schedule_Monitor();
}

void ISR_Build_Current_Machine_State_JSON() {

    // Wash_Machine_Panel (all LEDs status)
    // START
    // FINISH (NUMBER or WORKING)
    // LAST_CYCLE (NUMBER or WORKING)
}

void ISR_MOSFETs_Trigger() {

    if (MOSFET_1_STATE == MOSFET_ENABLED && ((millis() - MOSFET_1_TRIGGER_INIT_TIME) > MOSFET_TRIGGER_TIME)) {
        digitalWrite(MOSFET_BUTTON_START_PIN, LOW);

        Set_Firebase_Bool_at("Button_Start", false);
        if (!Get_Firebase_Bool_from("Button_Start")) { // ensures that the flag is clear
            MOSFET_1_STATE = MOSFET_DISABLED;
        }
    }

    if (MOSFET_2_STATE == MOSFET_ENABLED && ((millis() - MOSFET_2_TRIGGER_INIT_TIME) > MOSFET_TRIGGER_TIME)) {
        digitalWrite(MOSFET_BUTTON_TURBO_AGITACAO_PIN, LOW);

        Set_Firebase_Bool_at("Button_Start", false);
        if (!Get_Firebase_Bool_from("Button_Start")) { // ensures that the flag is clear
            MOSFET_2_STATE = MOSFET_DISABLED;
        }
    }

    if (MOSFET_3_STATE == MOSFET_ENABLED && ((millis() - MOSFET_3_TRIGGER_INIT_TIME) > MOSFET_TRIGGER_TIME)) {
        digitalWrite(MOSFET_BUTTON_TURBO_SECAGEM_PIN, LOW);

        Set_Firebase_Bool_at("Button_Start", false);
        if (!Get_Firebase_Bool_from("Button_Start")) { // ensures that the flag is clear
            MOSFET_3_STATE = MOSFET_DISABLED;
        }
    }

    if (MOSFET_4_STATE == MOSFET_ENABLED && ((millis() - MOSFET_4_TRIGGER_INIT_TIME) > MOSFET_TRIGGER_TIME)) {
        digitalWrite(MOSFET_BUTTON_REUTILIZAR_AGUA_PIN, LOW);

        Set_Firebase_Bool_at("Button_Start", false);
        if (!Get_Firebase_Bool_from("Button_Start")) { // ensures that the flag is clear
            MOSFET_4_STATE = MOSFET_DISABLED;
        }
    }

    if (MOSFET_5_STATE == MOSFET_ENABLED && ((millis() - MOSFET_5_TRIGGER_INIT_TIME) > MOSFET_TRIGGER_TIME)) {
        digitalWrite(MOSFET_BUTTON_AVANCAR_ETAPA_PIN, LOW);

        Set_Firebase_Bool_at("Button_Start", false);
        if (!Get_Firebase_Bool_from("Button_Start")) { // ensures that the flag is clear
            MOSFET_5_STATE = MOSFET_DISABLED;
        }
    }
}

void ISR_Server_Update() {
    Set_Firebase_String_at("/Device Calendar", Current_Date());
    Set_Firebase_String_at("/Device Clock", Current_Clock(WITHOUT_SECONDS));
}

Ticker ISR_GPIOs_Read_Controller(ISR_GPIOs_Read, 100, 0, MILLIS);
Ticker ISR_Display_Update_Controller(ISR_Display_Update, 100, 0, MILLIS);
Ticker ISR_Server_Monitor_Controller(ISR_Server_Monitor, 100, 0, MILLIS);
Ticker ISR_Server_Update_Controller(ISR_Server_Update, 100, 0, MILLIS);
Ticker ISR_MOSFETs_Trigger_Controller(ISR_MOSFETs_Trigger, 1, 0, MILLIS);

void Remote_Buttons_Monitor() {
    if (Get_Firebase_Bool_from("Button_Start") && MOSFET_1_STATE == MOSFET_DISABLED) {
        MOSFET_1_STATE = MOSFET_ENABLED;
        MOSFET_1_TRIGGER_INIT_TIME = millis();
        digitalWrite(MOSFET_BUTTON_START_PIN, HIGH);
    }

    if (Get_Firebase_Bool_from("Button_Next_Step") && MOSFET_2_STATE == MOSFET_DISABLED) {
        MOSFET_2_STATE = MOSFET_ENABLED;
        MOSFET_2_TRIGGER_INIT_TIME = millis();
        digitalWrite(MOSFET_BUTTON_TURBO_AGITACAO_PIN, HIGH);
    }

    if (Get_Firebase_Bool_from("Button_Turbo_Agitacao") && MOSFET_3_STATE == MOSFET_DISABLED) {
        MOSFET_3_STATE = MOSFET_ENABLED;
        MOSFET_3_TRIGGER_INIT_TIME = millis();
        digitalWrite(MOSFET_BUTTON_TURBO_SECAGEM_PIN, HIGH);
    }

    if (Get_Firebase_Bool_from("Button_Turbo_Secagem") && MOSFET_4_STATE == MOSFET_DISABLED) {
        MOSFET_4_STATE = MOSFET_ENABLED;
        MOSFET_4_TRIGGER_INIT_TIME = millis();
        digitalWrite(MOSFET_BUTTON_REUTILIZAR_AGUA_PIN, HIGH);
    }

    if (Get_Firebase_Bool_from("Button_Reutilizar_Agua") && MOSFET_5_STATE == MOSFET_DISABLED) {
        MOSFET_5_STATE = MOSFET_ENABLED;
        MOSFET_5_TRIGGER_INIT_TIME = millis();
        digitalWrite(MOSFET_BUTTON_AVANCAR_ETAPA_PIN, HIGH);
    }
}

void Remote_Schedule_Monitor() {

    Schedule_Time = isValid_Time(Get_Firebase_String_from("/SCHEDULE_TIME"));
}

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
    OLED_Print_Loading_Screen();

    ISR_GPIOs_Read_Controller.start();
    ISR_Display_Update_Controller.start();
    ISR_Server_Monitor_Controller.start();
    ISR_MOSFETs_Trigger_Controller.start();
    ISR_Server_Update_Controller.start();
}

void loop() {
    ISR_Display_Update_Controller.update();

    // ISR_GPIOs_Read_Controller.update();
     ISR_Server_Monitor_Controller.update();
    // ISR_MOSFETs_Trigger_Controller.update();
    // ISR_Server_Update_Controller.update();

    /*
        if (Current_Clock() == Schedule_Clock() && Last_Start_Time != "-1") {
            Set_Firebase_String_at("/START", "-1");
            Last_Start_Time = "-1";
            digitalWrite(BUZZER, HIGH);
            delay(1000);
            digitalWrite(BUZZER, LOW);
        }

        if (!Wash_Machine_State.LED_Ciclo_Iniciado && ScheduleClock != "-1") {
            Serial.println(" Clear_Active_Tasks()");
            Serial.println(Wash_Machine_State.LED_Ciclo_Iniciado);
            Clear_Active_Tasks();
        }
        */
}
