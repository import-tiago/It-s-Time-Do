#include "Board_Pins.h"
#include "Cloud.h"
#include "Firebase_Secrets.h"
#include "My_Persistent_Data.h"
#include "Network.h"
#include "OLED.h"
#include "WiFi_Secrets.h"
#include <Arduino.h>

#include <Ticker.h>

#define LED_CICLO_INICIADO_PIN 19
#define LED_TURBO_AGITACAO_PIN 19
#define LED_TURBO_SECAGEM_PIN 19
#define LED_REUTILIZAR_AGUA_PIN 19
#define LED_CENTRIFUGANDO_PIN 19
#define LED_ENXAGUANDO_PIN 19
#define LED_AGITACAO_CURTA_PIN 19
#define LED_AGITACAO_NORMAL_PIN 19
#define LED_AGITACAO_LONGA_PIN 19

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

    bool Button_Start = false;
    bool Button_Next_Step = false;
    bool Button_Turbo_Agitacao = false;
    bool Button_Turbo_Secagem = false;
    bool Button_Reutilizar_Agua = false;
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
    OLED_Print_Home_Screen();
}

void ISR_Server_Monitor() {

   if(Get_Firebase_Bool_from("Button_Start")){
       Set_Firebase_Bool_from("Button_Start", false);
       digitalWrite(LMOSFET_BUTTON_START_PIN, HIGH);

   }
    Wash_Machine.Button_Next_Step = Get_Firebase_Bool_from("Button_Next_Step");
    Wash_Machine.Button_Turbo_Agitacao = Get_Firebase_Bool_from("Button_Turbo_Agitacao");
    Wash_Machine.Button_Turbo_Secagem = Get_Firebase_Bool_from("Button_Turbo_Secagem");
    Wash_Machine.Button_Reutilizar_Agua = Get_Firebase_Bool_from("Button_Reutilizar_Agua");
}

void ISR_Build_Current_Machine_State_JSON() {

    // Wash_Machine_Panel (all LEDs status)
    // START
    // FINISH (NUMBER or WORKING)
    // LAST_CYCLE (NUMBER or WORKING)
}

Ticker ISR_GPIOs_Read_Controller(ISR_GPIOs_Read, 100, 0, MILLIS);
Ticker ISR_Display_Update_Controller(ISR_Display_Update, 100, 0, MILLIS);
Ticker ISR_Server_Monitor_Controller(ISR_Server_Monitor, 100, 0, MILLIS);

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
}

void loop() {
    ISR_GPIOs_Read_Controller.update();
    ISR_Display_Update_Controller.update();
    ISR_Server_Monitor_Controller.update();

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
}
