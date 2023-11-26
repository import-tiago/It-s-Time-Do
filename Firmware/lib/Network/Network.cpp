#include "Network.h"

bool init_wifi() {

	tftSprite.fillSprite(0x1924); // Berkeley Blue (RGB-565)
	tftSprite.setFreeFont(&FreeMono12pt7b);
	tftSprite.setTextWrap(true);

	tftSprite.setTextDatum(CC_DATUM);

	tftSprite.setTextColor(0xAEFC); // Uranian Blue (RGB-565)
	tftSprite.drawString("CONNECTING TO:", tftSprite.width() / 2, 15);

	tftSprite.setTextColor(YELLOW);
	tftSprite.drawString(WIFI_SSID, tftSprite.width() / 2, 45);

	TFT_Print();

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	tftSprite.setTextColor(0xAEFC); // Uranian Blue (RGB-565)

	uint8_t y = 80;
	uint8_t x = 4;
	uint8_t h = 20;
	tftSprite.setCursor(5, y);
	uint8_t progress = 0;


	uint16_t timeout = 60;

	while (WiFi.status() != WL_CONNECTED) {
		tftSprite.print(".");

		delay(1000);

		progress++;
		if (progress >= 17) {
			progress = 0;

			tftSprite.setCursor(x, y);
			tftSprite.fillRect(0, y - (h / 2), tftSprite.width(), h, 0x1924); // Berkeley Blue (RGB-565)
		}

		tftSprite.fillRect(tftSprite.width() - 40, tftSprite.height() - 40, 40, 40, 0x1924); // Berkeley Blue (RGB-565)
		tftSprite.drawString(String(--timeout), tftSprite.width() - 20, tftSprite.height() - 20);

		TFT_Print();

		if (!timeout)
			break;
	}

	if (!timeout) {
		wifi_connected = false;
		tftSprite.fillRect(tftSprite.width() - 40, tftSprite.height() - 40, 40, 40, 0x1924); // Berkeley Blue (RGB-565)
		tftSprite.setTextColor(YELLOW);
		tftSprite.drawString("CONNECTION FAIL!", tftSprite.width() / 2, y + 30);
		TFT_Print();
		delay(5000);
	}
	else
		wifi_connected = true;

	set_internal_esp_rtc_from_ntp("<-03>3"); // São Paulo/BR
	set_rtc_ic_from_internal_esp_rtc();
	print_local_time();

	tftSprite.println("\nSuccess!");
	TFT_Print();

	return wifi_connected;
}