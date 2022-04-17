#include "Board_Pins.h"
#include <Arduino.h>

void Board_Pins_Init() {

    Serial.begin(115200);

    pinMode(RELAY, OUTPUT);
    pinMode(WASH_MACHINE_POWER_LED, INPUT);
}