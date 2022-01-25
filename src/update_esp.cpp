#include "update_esp.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <esp_http_client.h>

#define HOST "https://github.com/momos11/MLED-Server/releases/download/Alpha/firmware.bin"

HTTPClient client;
// Global variables
int totalLength;       //total size of firmware
int currentLength = 0;


//download file https://github.com/momos11/MLED-Server/releases/download/Alpha/firmware.bin
void update_esp() {
    Serial.println("Starting update");
    WiFiClientSecure client;
//  client.setCACert(caCert);
    if (!client.connect(HOST, 443)) {
        Serial.println("connection failed");
        return;
    }
    Serial.println("connected");
    client.print(String("GET ") + HOST + " HTTP/1.1\r\n" + "Host: " + HOST + "\r\n" + "Connection: close\r\n\r\n");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("Content-Length:")) {
            totalLength = line.substring(16).toInt();
            Serial.println("Total Length: " + String(totalLength));
        }
        if (line == "\r") {
            Serial.println("headers received");
            break;
        }
    }
    if (!Update.begin(totalLength)) { //start with max available size
        Update.printError(Serial);
    }
    uint8_t buff[128] = {0};
    while (client.connected()) {
        if (currentLength < totalLength) {
            size_t bytesRead = client.read(buff, sizeof(buff));
            if (bytesRead > 0) {
                Update.write(buff, bytesRead);
                currentLength += bytesRead;
            }
        } else {
            break;
        }
    }
    if (Update.end()) {
        Serial.println("Update Success!");
    } else {
        Serial.println("Update Failed!");
    }
}


void updateInit() {
    // Connect to external web server
    client.begin(HOST);

    // Get file, just to check if each reachable
    int resp = client.GET();
    Serial.print("Response: ");
    Serial.println(resp);
    // If file is reachable, start downloading
    if (resp == 302) {
        // get length of document (is -1 when Server sends no Content-Length header)
        totalLength = client.getSize();
        // transfer to local variable
        int len = totalLength;
        // this is required to start firmware update process
        Update.begin(UPDATE_SIZE_UNKNOWN);
        Serial.printf("FW Size: %u\n", totalLength);
        // create buffer for read
        uint8_t buff[128] = {0};
        // get tcp stream
        WiFiClient *stream = client.getStreamPtr();
        // read all data from server
        Serial.println("Updating firmware...");
        while (client.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();
            if (size) {
                // read up to 128 byte
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                // pass to function
                updateFirmware(buff, c);
                if (len > 0) {
                    len -= c;
                }
            }
            delay(1);
        }
    } else {
        Serial.println(
                "Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
    }
    client.end();
}

void updateFirmware(uint8_t *data, size_t len) {
    Update.write(data, len);
    currentLength += len;
    // Print dots while waiting for update to finish
    Serial.print('.');
    // if current length of written firmware is not equal to total firmware size, repeat
    if (currentLength != totalLength) return;
    Update.end(true);
    Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
    // Restart ESP32 to see changes
    ESP.restart();
}