#include "api.h"
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include "cert.h"

enum {
    PRIMARY_COLOR,
    SECONDARY_COLOR,
    TOGGLE_STATE,
    LED_MODE,
    SPEED,
    BRIGHTNESS,
};

#define URL_fw_Bin "https://raw.githubusercontent.com/momos11/MLED-Server/main/firmware.bin"


Api::Api(Led *ledPointer) : server(80) {
    led = ledPointer;
    primaryColor = led->ws2812fx.getColors(0)[0];
    secondaryColor = led->ws2812fx.getColors(0)[1];
    toggleState = led->ws2812fx.isRunning();
    ledMode = led->ws2812fx.getMode();
    speed = led->ws2812fx.getSpeed();
    brightness = led->ws2812fx.getBrightness();
}

void Api::saveSettings(int key, uint32_t value) {
    Preferences preference;
    preference.begin("state", false);

    switch (key) {
        default:
            break;
        case PRIMARY_COLOR:
            preference.putUInt("primaryColor", value);
            break;
        case SECONDARY_COLOR:
            preference.putUInt("secondaryColor", value);
            break;
        case TOGGLE_STATE:
            preference.putBool("toggleState", value);
            break;
        case LED_MODE:
            preference.putUInt("ledMode", value);
            break;
        case SPEED:
            preference.putUInt("speed", value);
            break;
        case BRIGHTNESS:
            preference.putUInt("brightness", value);
            break;
    }
    preference.end();
}

void Api::setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
}

void Api::handleLedMode() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    if (doc["ledMode"].as<int>() != ledMode) {
        saveSettings(LED_MODE, doc["ledMode"].as<int>());
    }

    ledMode = doc["ledMode"].as<int>();
    server.send(200, "application/json", (String) ledMode);
    led->ws2812fx.setMode(ledMode);
}

void Api::handleLedModeGet() {
    DynamicJsonDocument doc(1024);
    doc["ledMode"] = ledMode;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleBrightness() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));


    if (doc["end"].as<bool>()) {
        Serial.println(doc["end"].as<bool>());
        saveSettings(BRIGHTNESS, doc["brightness"].as<int>());
    }

    brightness = doc["brightness"].as<int>();
    server.send(200, "application/json", (String) brightness);
    led->ws2812fx.setBrightness(brightness);
}

void Api::handleBrightnessGet() {
    DynamicJsonDocument doc(1024);
    doc["brightness"] = brightness;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleSpeed() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    if (doc["end"].as<bool>()) {
        saveSettings(SPEED, doc["speed"].as<int>());
    }

    speed = doc["speed"].as<int>();
    server.send(200, "application/json", (String) speed);
    led->ws2812fx.setSpeed(speed);
}

void Api::handleColor() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    if (doc["end"].as<bool>()) {
        saveSettings(PRIMARY_COLOR, doc["primaryColor"].as<uint32_t>());
        saveSettings(SECONDARY_COLOR, doc["secondaryColor"].as<uint32_t>());
    }

    primaryColor = doc["primaryColor"].as<uint32_t>();
    secondaryColor = doc["secondaryColor"].as<uint32_t>();
    doc.clear();
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    colors[0] = primaryColor;
    colors[1] = secondaryColor;
    led->ws2812fx.setColors(0, colors);
}

void Api::handleColorGet() {
    DynamicJsonDocument doc(1024);
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleToggle() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    if (doc["toggleState"].as<bool>() != toggleState) {
        saveSettings(TOGGLE_STATE, doc["toggleState"].as<bool>());
    }

    toggleState = doc["toggleState"].as<bool>();
    doc.clear();
    doc["toggleState"] = toggleState;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);

    if (!toggleState) {
        led->ws2812fx.stop();
    }
    if (toggleState) {
        led->ws2812fx.start();
        led->ws2812fx.setMode(ledMode);
    }
}

void Api::handleToggleGet() {
    DynamicJsonDocument doc(1024);
    doc["toggleState"] = toggleState;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleInformationGet() {
    primaryColor = led->ws2812fx.getColors(0)[0];
    secondaryColor = led->ws2812fx.getColors(0)[1];
    toggleState = led->ws2812fx.isRunning();
    ledMode = led->ws2812fx.getMode();
    speed = led->ws2812fx.getSpeed();
    brightness = led->ws2812fx.getBrightness();
    colors[0] = led->ws2812fx.getColors(0)[0];
    colors[1] = led->ws2812fx.getColors(0)[1];
    colors[2] = led->ws2812fx.getColors(0)[2];
    DynamicJsonDocument doc(1024);
    doc["toggleState"] = toggleState;
    doc["brightness"] = brightness;
    doc["ledMode"] = ledMode;
    doc["speed"] = speed;
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleReset() {
    if (!server.hasArg("plain")) {

        server.send(200, "application/json", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    if (doc["command"] == "reset_nvs" && doc["password"] == "b055684c-68d4-41e5-ac56-d140a2668cd4") {
        Preferences preferences;
        preferences.begin("WiFiCred", false);
        preferences.clear();
        preferences.end();
        preferences.begin("state", false);
        preferences.clear();
        preferences.end();
        ESP.restart();
    }
};

void Api::handleUpdate(){
    DynamicJsonDocument doc(1024);
    doc["message"] = "Is going to update!";
    char buffer[250];
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    setupOTA();
}

void Api::apiInit() {
    server.on("/ledMode", HTTP_POST, std::bind(&Api::handleLedMode, this));
    server.on("/ledMode", HTTP_GET, std::bind(&Api::handleLedModeGet, this));
    server.on("/brightness", HTTP_POST, std::bind(&Api::handleBrightness, this));
    server.on("/brightness", HTTP_GET, std::bind(&Api::handleBrightnessGet, this));
    server.on("/speed", HTTP_POST, std::bind(&Api::handleSpeed, this));
    server.on("/color", HTTP_POST, std::bind(&Api::handleColor, this));
    server.on("/color", HTTP_GET, std::bind(&Api::handleColorGet, this));
    server.on("/toggleState", HTTP_POST, std::bind(&Api::handleToggle, this));
    server.on("/toggleState", HTTP_GET, std::bind(&Api::handleToggleGet, this));
    server.on("/information", HTTP_GET, std::bind(&Api::handleInformationGet, this));
    server.on("/reset", HTTP_POST, std::bind(&Api::handleReset, this));
    server.on("/update", HTTP_GET, std::bind(&Api::handleUpdate, this));
}

void Api::setupOTA() {
    WiFiClientSecure client;
    Serial.println("Starting OTA");
    Serial.println("Set CACert");
    client.setCACert(rootCACertificate);
    Serial.println("CACert set");
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
    Serial.println("Update done");
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }}

void Api::startServer() {
    apiInit();
    setCrossOrigin();
    server.begin();
}

void Api::serverHandleClient() {
    server.handleClient();
}


