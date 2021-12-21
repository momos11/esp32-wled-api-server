#include "api.h"
#include <ArduinoJson.h>
#include <string>
#include <iostream>
#include <sstream>


Api::Api(Led *ledPointer) : server(80) {
    led = ledPointer;
    colorString = "";
    side = "";
    colorLeft = "";
    colorRight = "";
    toggleState = "";
    ledMode = 0;
    speed = 0;
    brightness = 0;
}

void Api::setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};


void Api::handleMode() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    ledMode = doc["ledMode"].as<int>();
    Serial.println(led->ws2812fx.getMode(ledMode));
    server.send(200, "text/plain", (String) ledMode);
    led->ws2812fx.setMode(ledMode);
}

void Api::handleBrightness() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    brightness = doc["brightness"].as<int>();
    server.send(200, "text/plain", (String) brightness);
    led->ws2812fx.setBrightness(brightness);
}

void Api::handleSpeed() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    speed = doc["speed"].as<int>();
    server.send(200, "text/plain", (String) speed);
    led->ws2812fx.setSpeed(speed);
}

void Api::handleColor() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    colorLeft = doc["colorSide"].as<String>();
    colorString = doc["color"].as<String>();
    server.send(200, "text/plain", colorString);
    std::string s(&colorString[1]);
    uint32_t value;
    std::istringstream iss(s);
    iss >> std::hex >> value;
    led->ws2812fx.setColor(value);
}

void Api::handleRightColor() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    colorRight = doc["colorSide"].as<String>();
    colorString = doc["color"].as<String>();
    server.send(200, "text/plain", colorString);
    uint32_t hexValueLeft = (uint32_t) strtol(&colorLeft[1], NULL, 16);
    uint32_t hexValueRight = (uint32_t) strtol(&colorString[1], NULL, 16);
    uint32_t colors[2] = {hexValueLeft, hexValueRight};
    led->ws2812fx.setColors(0, colors);
}

void Api::handleToggle() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    toggleState = doc["toggleState"].as<String>();
    server.send(200, "text/plain", toggleState);

    if (toggleState.equals("OFF")) {
        led->ws2812fx.stop();
    }
    if (toggleState.equals("ON")) {
        led->ws2812fx.start();
        led->ws2812fx.setMode(ledMode);
    }
}

void Api::handleToggleGet() {
    server.send(200, "text/plain", toggleState);
}

void Api::apiInit() {
    server.on("/ledMode", HTTP_POST, std::bind(&Api::handleMode, this));
    server.on("/brightness", HTTP_POST, std::bind(&Api::handleBrightness, this));
    server.on("/speed", HTTP_POST, std::bind(&Api::handleSpeed, this));
    server.on("/color", HTTP_POST, std::bind(&Api::handleColor, this));
    server.on("/colorRight", HTTP_POST, std::bind(&Api::handleRightColor, this));
    server.on("/toggleState", HTTP_POST, std::bind(&Api::handleToggle, this));
    server.on("/toggleState", HTTP_GET, std::bind(&Api::handleToggleGet, this));
}

void Api::startServer() {
    apiInit();
    setCrossOrigin();
    server.begin();
}

void Api::serverHandleClient() {
    server.handleClient();
}


