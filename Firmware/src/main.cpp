#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <Ticker.h>
#include <TridentTD_ESP32NVS.h>
#include <WiFi.h>
#include "WiFi_Secrets.h"
#include "Firebase_Secrets.h"

// Provide the token generation process info.
#include <addons/TokenHelper.h>
#define LED 2


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
    Serial.println("Check_OTA_Firmware_Update");
    char New_Firmware_Version[100] = {'\0'};
    memset(New_Firmware_Version, '\0', sizeof(New_Firmware_Version));

    if (Firebase.ready()) {
        Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, "test/firmware/bin/firmware.bin" /* remote file */) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {
            FileMetaInfo meta = fbdo.metaData();
            Serial.printf("Name: %s\n", meta.name.c_str());
            Serial.printf("Bucket: %s\n", meta.bucket.c_str());
            Serial.printf("contentType: %s\n", meta.contentType.c_str());
            Serial.printf("Size: %d\n", meta.size);
            Serial.printf("Generation: %lu\n", meta.generation);
            Serial.printf("Metageneration: %lu\n", meta.metageneration);
            Serial.printf("ETag: %s\n", meta.etag.c_str());
            Serial.printf("CRC32: %s\n", meta.crc32.c_str());
            Serial.printf("Token: %s\n", meta.downloadTokens.c_str());
            Serial.printf("Download URL: %s\n\n", fbdo.downloadURL().c_str());



            sprintf(New_Firmware_Version, String(meta.downloadTokens.c_str()).c_str());
            Serial.print("Firmware Token on Server: ");
            Serial.println(New_Firmware_Version);
            if (strcmp(Current_Firmware_Version, New_Firmware_Version) != 0) {
                Serial.println("New firmware version available");
                NVS.setObject("FW", &New_Firmware_Version, sizeof(New_Firmware_Version));
                Download_New_Firmware_by_OTA();
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    NVS.begin();

    if (String((char *)NVS.getObject("FW")).length() > 1)
        strcpy(Current_Firmware_Version, String((char *)NVS.getObject("FW")).c_str());

    if (!strlen(Current_Firmware_Version))
        sprintf(Current_Firmware_Version, "-1");

    Serial.print("FIRMWARE_VERSION: ");
    Serial.println(Current_Firmware_Version);



    pinMode(LED, OUTPUT);

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

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    /* Assign download buffer size in byte */
    // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

      Checks_OTA_Firmware_Update();
}

void loop() {
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
}