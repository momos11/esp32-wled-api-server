#include "api.h"
#include <ArduinoJson.h>

DynamicJsonDocument doc(1024);
char buffer[250];

Api::Api(Led *ledPointer) : server(80) {
    led = ledPointer;
    primaryColor = led->ws2812fx.getColor(0);
    secondaryColor = led->ws2812fx.getColor(1);
    toggleState = led->ws2812fx.isRunning();
    ledMode = led->ws2812fx.getMode();
    speed = led->ws2812fx.getSpeed();
    brightness = led->ws2812fx.getBrightness();
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
}

void Api::startServer() {
    apiInit();
    setCrossOrigin();
    server.begin();
}

void Api::serverHandleClient() {
    server.handleClient();
}


