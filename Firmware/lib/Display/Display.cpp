#include "Display.h"

TFT_eSprite tftSprite = TFT_eSprite(&M5.Lcd);

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
	return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

void build_wifi_status() {

	if (wifi_connected) {

		int8_t dBm = 0;
		uint8_t quality_percentage = 0;

		tftSprite.setTextColor(0xEFBF); // Alice Blue (RGB-565)
		tftSprite.setFreeFont(&FreeMono9pt7b);

		dBm = WiFi.RSSI();

		if (dBm <= -100)
			quality_percentage = 0;
		else if (dBm >= -50)
			quality_percentage = 100;
		else
			quality_percentage = 2 * (dBm + 100);

		tftSprite.setCursor(45, 14);
		char buf[10];
		sprintf(buf, "%ddBm", dBm);
		tftSprite.print(buf);

		tftSprite.setCursor(45, 30);
		sprintf(buf, "%u%%", quality_percentage);
		tftSprite.print(buf);


		const uint8_t x = 7;
		const uint8_t y = 17;
		const uint8_t w = 5;
		const uint8_t h = 6;
		const uint8_t radius = 1;
		const uint16_t high_color = 0x7FDF; // Electric blue (RGB-565)
		const uint16_t low_color = DARKGREY;

		if (dBm > -50 & !dBm == 0)
			tftSprite.fillRoundRect(x + 24, y - 6, w, h + 6, radius, high_color);
		else
			tftSprite.fillRoundRect(x + 24, y - 6, w, h + 6, radius, low_color);


		if (dBm > -70 & !dBm == 0)
			tftSprite.fillRoundRect(x + 16, y - 4, w, h + 4, radius, high_color);
		else
			tftSprite.fillRoundRect(x + 16, y - 4, w, h + 4, radius, low_color);


		if (dBm > -80 & !dBm == 0)
			tftSprite.fillRoundRect(x + 8, y - 2, w, h + 2, radius, high_color);
		else
			tftSprite.fillRoundRect(x + 8, y - 2, w, h + 2, radius, low_color);


		if (dBm > -90 & !dBm == 0)
			tftSprite.fillRoundRect(x, y, w, h, radius, high_color);
		else
			tftSprite.fillRoundRect(x, y, w, h, radius, low_color);

	}
	else {

		tftSprite.setTextColor(0xEFBF); // Alice Blue (RGB-565)
		tftSprite.setFreeFont(&Orbitron_Light_24);
		tftSprite.setTextDatum(CL_DATUM);

		static bool blinky = false;

		static uint32_t t0 = millis();
		if (millis() - t0 > 500) {
			t0 = millis();
			blinky = !blinky;
		}
		if (blinky)
			tftSprite.drawString("OFFLINE", 7, 13);
	}
}

void build_status_bar() {

	char data[5];

	tftSprite.fillRect(0, 0, 240, 35, 0x018C); // Berkeley Blue (RGB-565)
	tftSprite.setTextColor(0xEFBF); // Alice Blue (RGB-565)
	tftSprite.setFreeFont(&Orbitron_Light_24);

	static uint16_t last_bat_percentage = 0;
	static uint16_t bat_percentage = 100;

	last_bat_percentage = (uint16_t)mapFloat(M5.Axp.GetBatVoltage(), 3.00F, 4.10F, 0.0F, 100.00F);

	if (last_bat_percentage > 100)
		last_bat_percentage = 100;

	if (last_bat_percentage < bat_percentage)
		bat_percentage = last_bat_percentage;

	sprintf(data, "%u%%", bat_percentage);

	tftSprite.setTextDatum(CR_DATUM);
	tftSprite.drawString(data, tftSprite.width() - 5, 13);

	build_wifi_status();
}

void build_current_time() {

	tftSprite.setTextColor(0xEFBF); // Alice Blue (RGB-565)
	tftSprite.setFreeFont(&Orbitron_Light_32);

	tftSprite.setTextDatum(CC_DATUM);
	tftSprite.drawString(Current_Clock(FULL), tftSprite.width() / 2, tftSprite.height() / 2);
}

void build_next_task() {

	char data[20];

	tftSprite.setFreeFont(&Orbitron_Light_24);

	if (Next_Task != Washing_Machine.FREE && Next_Task != Washing_Machine.FAIL) {

		tftSprite.setTextColor(0xAEDE); // Uranian Blue (RGB-565)

		int hour = Next_Task.substring(0, Next_Task.indexOf(":")).toInt();
		int min = Next_Task.substring(Next_Task.indexOf(":") + 1).toInt();
		sprintf(data, "Next task: %02d:%02d", hour, min);
	}
	else {
		tftSprite.setTextColor(0x07E0); // Green (RGB-565)
		sprintf(data, "Next task: FREE");
	}

	tftSprite.setTextDatum(BC_DATUM);
	tftSprite.drawString(data, tftSprite.width() / 2, tftSprite.height() - 7);
}

void TFT_Init() {
	M5.Lcd.setRotation(1);
	tftSprite.createSprite(M5.Lcd.width(), M5.Lcd.height());
	tftSprite.setRotation(1);
}

void TFT_Build_Home_Screen(String _Schedule_Time, String Firmware_Version) {

	build_status_bar();

	build_current_time();

	build_next_task();
}

void TFT_Clear() {
	tftSprite.fillSprite(0x18E3); //Eerie black (RGB-565)
}

void TFT_Print() {
	tftSprite.pushSprite(0, 0);
}

void TFT_Build_Working_Screen(String current_duration, String Firmware_Version) {

	build_status_bar();
	TFT_Print_Current_Task_Duration(current_duration);
}

void TFT_Print_Current_Task_Duration(String value) {

	tftSprite.setTextColor(0xEFBF); // Alice Blue (RGB-565)
	tftSprite.setFreeFont(&Orbitron_Light_32);

	tftSprite.setTextDatum(CC_DATUM);
	tftSprite.drawString(value, tftSprite.width() / 2, 60);
	tftSprite.setFreeFont(&Orbitron_Light_24);
	tftSprite.setTextColor(GREEN);
	tftSprite.drawString("WORKING...", tftSprite.width() / 2, 100);
}

void TFT_Wait_Task_Initialize_Screen(uint8_t timeout) {

	build_status_bar();

	tftSprite.setFreeFont(&Orbitron_Light_24);
	tftSprite.setTextColor(ORANGE);
	tftSprite.setTextDatum(CC_DATUM);
	tftSprite.drawString("WAITING", tftSprite.width() / 2, 55);
	tftSprite.drawString("LED POWER ON", tftSprite.width() / 2, 80);
	tftSprite.fillRect(tftSprite.width() - 40, tftSprite.height() - 40, 40, 40, 0x18E3); //Eerie black (RGB-565)
	tftSprite.drawString(String(timeout), tftSprite.width() - 20, tftSprite.height() - 20);
}