#include "Firebase_Secrets.h"
#include "WiFi_Secrets.h"
#include <Arduino.h>
#include <Firebase_ESP_Client.h>

#include <TridentTD_ESP32NVS.h>
#include <WiFi.h>

#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <SPI.h>
#include <Wire.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>

RTC_DS3231 RTC;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
#define LED 2
#define BUZZER 4
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void Display_Check_OTA_Firmware_Update();
void OLED_OTA_Progress(int status);
void Display_WiFi_Connecting();
void Display_Firebase_Connecting();

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

char Current_Firmware_Version[100] = {'\0'};

// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info) {

    if (info.status == fb_esp_fcs_download_status_init) {

        Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
    } else if (info.status == fb_esp_fcs_download_status_download) {
        Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
        OLED_OTA_Progress((int)info.progress);
    } else if (info.status == fb_esp_fcs_download_status_complete) {
        Serial.println("Update firmware completed.");
        Serial.println();
        Serial.println("Restarting...\n\n");
        delay(2000);
        ESP.restart();
    } else if (info.status == fb_esp_fcs_download_status_error) {
        Serial.printf("Download firmware failed, %s\n", info.errorMsg.c_str());
    }
}

void Download_New_Firmware_by_OTA() {

    if (!Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID, "test/firmware/bin/firmware.bin", fcsDownloadCallback))
        Serial.println(fbdo.errorReason());
}

void Checks_OTA_Firmware_Update() {

    Display_Check_OTA_Firmware_Update();

    Serial.println("Check_OTA_Firmware_Update");
    char New_Firmware_Version[100] = {'\0'};
    memset(New_Firmware_Version, '\0', sizeof(New_Firmware_Version));

    if (Firebase.ready()) {
        Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, "test/firmware/bin/firmware.bin") ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {

            FileMetaInfo meta = fbdo.metaData();

            sprintf(New_Firmware_Version, String(meta.downloadTokens.c_str()).c_str());
            Serial.print("Firmware Token on Server: ");
            Serial.println(New_Firmware_Version);
            if (strcmp(Current_Firmware_Version, New_Firmware_Version) != 0) {
                Serial.println("New firmware version available");

                Download_New_Firmware_by_OTA();
                NVS.setObject("FW", &New_Firmware_Version, sizeof(New_Firmware_Version));
            } else
                Serial.println("Firmware no needs update.");
        }
    }
}

void Display_Check_OTA_Firmware_Update() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(15, 5);
    display.print("CHECKING");
    display.setCursor(15, 25);
    display.print("FIRMWARE");
    display.setCursor(25, 45);
    display.print("UPDATE");
    display.display();
}

void Display_WiFi_Connecting() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5, 5);
    display.print("Wi-Fi Connecting...");
    display.setCursor(5, 20);
    display.print(WIFI_SSID);
    display.display();
}

void Display_Firebase_Connecting() {

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5, 5);
    display.print("Firebase");
    display.setCursor(5, 20);
    display.print("Connecting...");
    display.display();
}

void OLED_OTA_Progress(int status) {
    display.clearDisplay();
    display.setTextColor(WHITE);

    display.setTextSize(2);
    display.setCursor(10, 5);
    display.print("UPDATING");

    display.setTextSize(4);
    display.setCursor(10, 30);
    display.print(String(status) + "%");
    display.display();
}

void setup() {
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    if (!RTC.begin()) {
        Serial.println("RTC Init Fail");
        while (1) {
            ;
        }
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED Init Fail"));
        while (1) {
            ;
        }
    }

    // RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

    NVS.begin();

    if (String((char *)NVS.getObject("FW")).length() > 1)
        strcpy(Current_Firmware_Version, String((char *)NVS.getObject("FW")).c_str());

    if (!strlen(Current_Firmware_Version))
        sprintf(Current_Firmware_Version, "-1");

    Serial.print("FIRMWARE_VERSION: ");
    Serial.println(Current_Firmware_Version);

    pinMode(BUZZER, OUTPUT);

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

    Display_Firebase_Connecting();
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // Assign the api key (required)
    config.api_key = API_KEY;

    // Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Assign download buffer size in byte
    // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    Checks_OTA_Firmware_Update();

    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);
    delay(50);
}

void loop() {

    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);

    DateTime now = RTC.now();
    char dateBuffer[12];

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(2, 5);
    sprintf(dateBuffer, "%02u/%02u/%04u ", now.day(), now.month(), now.year());
    display.println(dateBuffer);

    display.setCursor(12, 25);
    sprintf(dateBuffer, "%02u:%02u:%02u ", now.hour(), now.minute(), now.second());
    display.println(dateBuffer);
    display.setCursor(15, 45);
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);

    display.display();
}