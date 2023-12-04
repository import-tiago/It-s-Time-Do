#include "my_hal.h"

void init_gpios() {

	M5.begin();

	pinMode(TARGET_DEVICE_POWER_LED, INPUT);
	pinMode(TARGET_DEVICE_POWER_SWITCH, OUTPUT);
	digitalWrite(TARGET_DEVICE_POWER_SWITCH, LOW);

	delay(2000);
}