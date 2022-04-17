#include "OLED.h"
#include "WiFi_Secrets.h"
#include <Arduino.h>
#include <WiFi.h>

void WiFi_Init() {
    Display_WiFi_Connecting();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {

        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}