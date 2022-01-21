#include "api.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Update.h>

DynamicJsonDocument doc(1024);
char buffer[250];

enum {
    PRIMARY_COLOR,
    SECONDARY_COLOR,
    TOGGLE_STATE,
    LED_MODE,
    SPEED,
    BRIGHTNESS,
};


Api::Api(Led *ledPointer) : server(80) {
    led = ledPointer;
    primaryColor = led->ws2812fx.getColor(0);
    secondaryColor = led->ws2812fx.getColor(1);
    toggleState = led->ws2812fx.isRunning();
    ledMode = led->ws2812fx.getMode();
    speed = led->ws2812fx.getSpeed();
    brightness = led->ws2812fx.getBrightness();
}

void Api::saveSettings(int key, int value) {
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
            preference.putInt("ledMode", value);
            break;
        case SPEED:
            preference.putInt("speed", value);
            break;
        case BRIGHTNESS:
            preference.putInt("brightness", value);
            break;
    }
    preference.end();
}

void Api::setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

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
    doc.clear();
    doc["ledMode"] = ledMode;
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

    if(doc["brightness"].as<int>() != brightness && doc["end"].as<bool>()) {
        saveSettings(BRIGHTNESS, doc["brightness"].as<int>());
    }

    brightness = doc["brightness"].as<int>();
    server.send(200, "application/json", (String) brightness);
    led->ws2812fx.setBrightness(brightness);
}

void Api::handleBrightnessGet() {
    doc.clear();
    doc["brightness"] = brightness;
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

    if(doc["speed"].as<int>() != speed && doc["end"].as<bool>()) {
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

    if (doc["primaryColor"].as<int>() != primaryColor && doc["end"].as<bool>()) {
        saveSettings(PRIMARY_COLOR, doc["primaryColor"].as<int>());
    }

    if (doc["secondaryColor"].as<int>() != secondaryColor && doc["end"].as<bool>()) {
        saveSettings(SECONDARY_COLOR, doc["secondaryColor"].as<int>());
    }

    primaryColor = doc["primaryColor"].as<int>();
    secondaryColor = doc["secondaryColor"].as<int>();
    doc.clear();
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    led->ws2812fx.setColor(0, primaryColor);
    led->ws2812fx.setColor(1, primaryColor);
}

void Api::handleColorGet() {
    doc.clear();
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
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

    if (doc["toggle"].as<bool>() != toggleState) {
        saveSettings(TOGGLE_STATE, doc["toggleState"].as<bool>());
    }

    toggleState = doc["toggleState"].as<bool>();
    doc.clear();
    doc["toggleState"] = toggleState;
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
    doc.clear();
    doc["toggleState"] = toggleState;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
}

void Api::handleInformationGet() {
    primaryColor = led->ws2812fx.getColor(0);
    secondaryColor = led->ws2812fx.getColor(1);
    toggleState = led->ws2812fx.isRunning();
    ledMode = led->ws2812fx.getMode();
    speed = led->ws2812fx.getSpeed();
    brightness = led->ws2812fx.getBrightness();
    doc.clear();
    doc["toggleState"] = toggleState;
    doc["brightness"] = brightness;
    doc["ledMode"] = ledMode;
    doc["speed"] = speed;
    doc["primaryColor"] = primaryColor;
    doc["secondaryColor"] = secondaryColor;
    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
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
    setupOTA();
}

void Api::setupOTA(){
    server.on("/update", HTTP_POST, [this]() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, [this]() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });
}

void Api::startServer() {
    apiInit();
    setCrossOrigin();
    server.begin();
}

void Api::serverHandleClient() {
    server.handleClient();
}


