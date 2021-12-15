# 1 "C:\\Users\\mauri\\AppData\\Local\\Temp\\tmp8g3qpa3u"
#include <Arduino.h>
# 1 "D:/GitHub/esp32-wled-api-server/src/main.ino"

#include <Arduino.h>
#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>





#include <ArduinoJson.h>
#include <stdlib.h>
#include <string.h>
#include <base64encode.h>

#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <Preferences.h>


const char compileDate[] = __DATE__ " " __TIME__;


char apName[] = "LED-Strip";




bool usePrimAP = true;

bool hasCredentials = false;

volatile bool isConnected = false;

bool connectionStatusChanged = false;
void createName();
void initBLE();
void gotIP(system_event_id_t event);
void lostCon(system_event_id_t event);
void connectWiFi();
void setup();
void loop();
void setCrossOrigin();
void handleMode();
void handleBrightness();
void handleSpeed();
void handleColor();
void handleRightColor();
void handleToggle();
void wifiConnect();
void serverInit();
void setup2();
void loop2();
#line 42 "D:/GitHub/esp32-wled-api-server/src/main.ino"
void createName() {
    uint8_t baseMac[6];

    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);

    sprintf(apName, "LED-Strip-%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4],
            baseMac[5]);
}


#define SERVICE_UUID "f9c521f6-0f14-4499-8f76-43116b40007d"
#define WIFI_UUID "23456f8d-4aa7-4a61-956b-39c9bce0ff00"


String ssid;

String password;


BLECharacteristic *pCharacteristicWiFi;

BLEAdvertising *pAdvertising;

BLEService *pService;

BLEServer *pServer;





StaticJsonDocument<100> jsonBuffer;





class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        Serial.println("BLE client connected");
    };

    void onDisconnect(BLEServer *pServer) {
        Serial.println("BLE client disconnected");
        pAdvertising->start();
    }
};





class MyCallbackHandler : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() == 0) {
            return;
        }
        Serial.println("Received over BLE: " + String((char *) &value[0]));


        StaticJsonDocument<256> jsonIn;
        deserializeJson(jsonIn, String((char *) &value[0]));
        ssid = jsonIn["ssid"].as<String>();
        password = jsonIn["password"].as<String>();

        Preferences preferences;
        preferences.begin("WiFiCred", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.putBool("valid", true);
        preferences.end();

        Serial.println("Received over bluetooth:");
        Serial.println("primary SSID: " + ssid + " password: " + password);
        connectionStatusChanged = true;
        hasCredentials = true;
# 140 "D:/GitHub/esp32-wled-api-server/src/main.ino"
        jsonBuffer.clear();
    };

    void onRead(BLECharacteristic *pCharacteristic) {
        Serial.println("BLE onRead request");
        String credentials;


        StaticJsonDocument<50> jsonOut;
        jsonOut["ssid"] = ssid;
        jsonOut["password"] = password;

        serializeJson(jsonOut, credentials);

        String ipAdress = WiFi.localIP().toString();
        std::string stdSTring = (ipAdress.c_str());
        pCharacteristicWiFi->setValue(stdSTring);
        jsonBuffer.clear();
    }
};






void initBLE() {

    BLEDevice::init(apName);
    BLEDevice::setPower(ESP_PWR_LVL_P7);


    pServer = BLEDevice::createServer();


    pServer->setCallbacks(new MyServerCallbacks());


    pService = pServer->createService(BLEUUID(SERVICE_UUID), 20);


    pCharacteristicWiFi = pService->createCharacteristic(
            BLEUUID(WIFI_UUID),

            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());


    pService->start();


    pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}


void gotIP(system_event_id_t event) {
    isConnected = true;
    connectionStatusChanged = true;
}


void lostCon(system_event_id_t event) {
    isConnected = false;
    connectionStatusChanged = true;
}




void connectWiFi() {

    WiFi.onEvent(gotIP, SYSTEM_EVENT_STA_GOT_IP);

    WiFi.onEvent(lostCon, SYSTEM_EVENT_STA_DISCONNECTED);

    WiFi.disconnect(true);
    WiFi.enableSTA(true);
    WiFi.mode(WIFI_STA);

    Serial.println();
    Serial.print("Start connection to ");
    if (usePrimAP) {
        Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
    }
}

void setup() {

    createName();


    Serial.begin(115200);

    Serial.print("Build: ");
    Serial.println(compileDate);

    Preferences preferences;
    preferences.begin("WiFiCred", false);
    bool hasPref = preferences.getBool("valid", false);
    if (hasPref) {
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
        if (ssid.equals("")
            || password.equals("")) {
            Serial.println("Found preferences but credentials are invalid");
        } else {
            Serial.println("Read from preferences:");
            Serial.println("primary SSID: " + ssid + " password: " + password);
            hasCredentials = true;
        }
    } else {
        Serial.println("Could not find preferences, need send data over BLE");
    }
    preferences.end();


    initBLE();

    if (hasCredentials) {


        connectWiFi();
    }
}

void loop() {
    if (connectionStatusChanged) {
        if (isConnected) {
            Serial.print("Connected to AP: ");
            Serial.print(WiFi.SSID());
            Serial.print(" with IP: ");
            Serial.print(WiFi.localIP());
            Serial.print(" RSSI: ");
            Serial.println(WiFi.RSSI());
        } else {
            if (hasCredentials) {
                Serial.println("Lost WiFi connection");


                connectWiFi();
            }
        }
        connectionStatusChanged = false;
    }
}
# 1 "D:/GitHub/esp32-wled-api-server/src/mainTest.ino"
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

    Serial.print("Connecting to ");

    Serial.println(" ...");
    while (WiFi.status() != WL_CONNECTED) {
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