//
// Created by mauri on 14.12.2021.
//

#ifndef ESP32_WLED_API_SERVER_WLAN_SETUP_H
#define ESP32_WLED_API_SERVER_WLAN_SETUP_H

// List of Service and Characteristic UUIDs
#define SERVICE_UUID  "f9c521f6-0f14-4499-8f76-43116b40007d"
#define WIFI_UUID     "23456f8d-4aa7-4a61-956b-39c9bce0ff00"

#include <BLEServer.h>
#include <Arduino.h>
#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <ArduinoJson.h>
// Includes for BLE
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <Preferences.h>

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer);

    void onDisconnect(BLEServer *pServer);

public:
    MyServerCallbacks();
};

class MyCallbackHandler : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic);

    void onRead(BLECharacteristic *pCharacteristic);

public:
    MyCallbackHandler();
};

void initBLE();

void gotIP();

void lostCon();

bool connectWiFi();

void setupWlan();

void createName();

void waitForBluetoothConnection();

#endif //ESP32_WLED_API_SERVER_WLAN_SETUP_H
