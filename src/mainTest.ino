#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <WS2812FX.h>
#include <iostream>
#include <sstream>

#define LEDPIN 12
#define NUMPIXELS 60
#define TIMER_MS 500

//const char *ssid = "57Dps3P";
//const char *password = "3p7SD#m$87sa5k?=7HG";
WebServer server(80);
int ledMode = 0;
int speed = 0;
int brightness = 0;
String colorString = "";
String side = "";
String colorLeft = "";
String colorRight = "";
String toggleState = "";
int rgb[3] = {};
unsigned long last_change = 0;
unsigned long now = 0;

WS2812FX ws2812fx = WS2812FX(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

void setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

int *colorConverter(String hexString) {
    unsigned int hexValue = (int) strtol(&hexString[1], NULL, 16);
    Serial.println(hexValue);
    int red = hexValue >> 16;
    int green = hexValue >> 8 & 0xFF;
    int blue = hexValue & 0xFF;

    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
    Serial.println(red);
    Serial.println(green);
    Serial.println(blue);
    return rgb;
}

void handleMode(){
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    ledMode = doc["ledMode"].as<int>();
    Serial.println(ws2812fx.getMode(ledMode));
    server.send(200, "text/plain", (String) ledMode);
    ws2812fx.setMode(ledMode);
}

void handleBrightness(){
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    brightness = doc["brightness"].as<int>();
    server.send(200, "text/plain", (String) brightness);
    ws2812fx.setBrightness(brightness);
}

void handleSpeed() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    speed = doc["speed"].as<int>();
    server.send(200, "text/plain", (String) speed);
    ws2812fx.setSpeed(speed);
}

void handleColor() {
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
    ws2812fx.setColor(value);
}

void handleRightColor() {
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
    uint32_t colors [2] = {hexValueLeft, hexValueRight};
    ws2812fx.setColors(0,colors);
}

void handleToggle() {
    if (!server.hasArg("plain")) {

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    toggleState = doc["toggleState"].as<String>();
    server.send(200, "text/plain", toggleState);

    if (toggleState.equals("OFF")) {
        ws2812fx.stop();
    }
    if (toggleState.equals("ON")){
        ws2812fx.start();
        ws2812fx.setMode(ledMode);
    }
}

void wifiConnect() {
//    WiFi.begin(ssid, password); // Connect to the network
    Serial.print("Connecting to ");
//    Serial.print(ssid);
    Serial.println(" ...");
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(2000);
        Serial.print(WiFi.status());
        Serial.print('\n');
    }
    Serial.println('\n');
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
}


void serverInit(){
    server.on("/ledMode", HTTP_POST, handleMode);
    server.on("/brightness", HTTP_POST, handleBrightness);
    server.on("/speed", HTTP_POST, handleSpeed);
    server.on("/color", HTTP_POST, handleColor);
    server.on("/colorRight", HTTP_POST, handleRightColor);
    server.on("/toggle", HTTP_POST, handleToggle);
}


void setup2() {
    Serial.begin(115200);

    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    delay(10);
    Serial.println('\n');

    wifiConnect();

    serverInit();

    server.begin();
    ws2812fx.init();
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(1000);
    ws2812fx.setColor(0x007BFF);
    ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    ws2812fx.start();
}

void loop2() {
    server.handleClient();
    ws2812fx.service();
    if(millis() - last_change > TIMER_MS) {
        last_change = now;
    }
}
