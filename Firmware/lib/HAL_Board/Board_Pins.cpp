#include "Board_Pins.h"

void Board_Pins_Init() {

	 M5.begin();

	pinMode(WASHING_MACHINE_POWER_LED, INPUT);
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);

	delay(2000);
}