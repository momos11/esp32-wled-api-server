#include <Arduino.h>
#include "wlan_setup.h"
#include "api.h"
#include "ws_led.h"
#include "update_esp.h"

#define TIMER_MS 400

unsigned long last_change = 0;
unsigned long now = 0;
Led led;
Led *ledPointer = &led;
Api api(ledPointer);

void setup() {
    //connects to WLAN if credentials are stored
//    Serial.println("Initializing BLE...");
//    BLEDevice::init("Test");
//    Serial.println("BLE initialized");
    //setupWlan();

//    BLECharacteristic *pCharacteristicWiFi;
//    BLEAdvertising *pAdvertising;
//    BLEService *pService;
//    BLEServer *pServer;

//    Serial.println("Initializing BLE...");
//    BLEDevice::init("apName");
//    Serial.println("BLE initialized");
//    BLEDevice::setPower(ESP_PWR_LVL_P7);
//
//    // Create BLE Server
//    pServer = BLEDevice::createServer();
//
//    // Set server callbacks
//    pServer->setCallbacks(new MyServerCallbacks());
//
//    // Create BLE Service
//    pService = pServer->createService(BLEUUID(SERVICE_UUID));
//
//    // Create BLE Characteristic for WiFi settings
//    pCharacteristicWiFi = pService->createCharacteristic(
//            BLEUUID(WIFI_UUID),
//            NIMBLE_PROPERTY::READ |
//            NIMBLE_PROPERTY::WRITE |
//            NIMBLE_PROPERTY::NOTIFY
//    );
//    pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());
//
//    pCharacteristicWiFi->addDescriptor(new NimBLE2904());
//    // Start the service
//    pService->start();
//
//    // Start advertising
//    pAdvertising = pServer->getAdvertising();
//    pAdvertising->start();
//    Serial.println("BLE finished");
    //update_esp();
//    pinMode(LEDPIN, OUTPUT);
//    digitalWrite(LEDPIN, LOW);
//    delay(10);
//    led.startLed();
//    api.startServer();
    Serial.println("LED started");
}

void loop() {
//    api.serverHandleClient();
//    led.ledService();
//    if (millis() - last_change > TIMER_MS) {
//        last_change = now;
//    }
}
