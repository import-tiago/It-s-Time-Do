#include "Board_Pins.h"

void init_gpios() {

	 M5.begin();

	pinMode(WASHING_MACHINE_POWER_LED, INPUT);
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);

	delay(2000);
}