#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <Arduino.h>

#define LED 36
#define RELAY 23



// WASH MACHINE LEDs
#define LED_CICLO_INICIADO_PIN 36
#define LED_TURBO_AGITACAO_PIN 39
#define LED_TURBO_SECAGEM_PIN 34
#define LED_REUTILIZAR_AGUA_PIN 35
#define LED_CENTRIFUGANDO_PIN 32
#define LED_ENXAGUANDO_PIN 33
#define LED_AGITACAO_CURTA_PIN 25
#define LED_AGITACAO_NORMAL_PIN 26
#define LED_AGITACAO_LONGA_PIN 27

#define NUMBER_OF_BUTTONS 9
//------------------------------------------

// WASH MACHINE BUTTONS
#define MOSFET_BUTTON_START_PIN 14
#define MOSFET_BUTTON_TURBO_AGITACAO_PIN 12
#define MOSFET_BUTTON_TURBO_SECAGEM_PIN 13
#define MOSFET_BUTTON_REUTILIZAR_AGUA_PIN 23
#define MOSFET_BUTTON_AVANCAR_ETAPA_PIN 15

#define NUMBER_OF_BUTTONS 5
#define MOSFET_TRIGGER_TIME 500 // 500 mili-seconds
//------------------------------------------





// PROTOTYPES
void Board_Pins_Init();

#endif // HAL_BOARD_H
