#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
#include <WS2812FX.h>

#define LEDPIN 12
#define NUMPIXELS 60
#define TIMER_MS 5000

const char *ssid = "57Dps3P";
const char *password = "3p7SD#m$87sa5k?=7HG";
WebServer server(80);
String lastState = "";
int ledState = HIGH;
String ledMode = "";
String colorString = "";
String side = "";
String colorLeft = "";
String colorRight = "";
String toggleState = "";
String brightness = "";
int rgb[3] = {};
unsigned long last_change = 0;
unsigned long now = 0;

WS2812FX ws2812fx = WS2812FX(NUMPIXELS, LEDPIN, NEO_RGB + NEO_KHZ800);


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

void handlePattern() {

    if (ledMode == "RAINBOW") {
        Serial.println("RAINBOW");
        ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    }


    if (ledMode == "COLOR") {
        Serial.println(colorString);
        Serial.println(side);

        colorConverter(colorString);

        if (side == "color1") {

        }

        if (side == "color2") {

        }
    }
}


void handleSubmit() { //Handler for the body path
    if (!server.hasArg("plain")) { //Check if body received

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    ledMode = doc["ledMode"].as<String>();
    colorString = doc["color"].as<String>();
    side = doc["side"].as<String>();
    brightness = doc["brightness"].as<String>();
    String message = ledMode;
    Serial.println(ledMode);
    handlePattern();

    server.send(200, "text/plain", message);
}

void handleGet() {

}

void wifiConnect() {
    WiFi.begin(ssid, password); // Connect to the network
    Serial.print("Connecting to ");
    Serial.print(ssid);
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



void setup() {
    Serial.begin(115200);

    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    delay(10);
    Serial.println('\n');

    wifiConnect();

    server.on("/submit", HTTP_POST, handleSubmit);
    server.on("/get", HTTP_POST, handleGet);

    server.begin();
    ws2812fx.init();
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(1000);
    ws2812fx.setColor(0x007BFF);
    ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    ws2812fx.start();
}

void loop() {
    server.handleClient();
    now = millis();

    ws2812fx.setSpeed(4000);
    ws2812fx.service();

    if(now - last_change > TIMER_MS) {
        ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
        last_change = now;
    }
}
