#include "Board_Pins.h"
#include <Arduino.h>



void Board_Pins_Init() {
    Serial.begin(115200);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);

    pinMode(LED_CICLO_INICIADO_PIN, INPUT);
}