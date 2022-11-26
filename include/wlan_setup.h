//
// Created by mauri on 14.12.2021.
//

#ifndef ESP32_WLED_API_SERVER_WLAN_SETUP_H
#define ESP32_WLED_API_SERVER_WLAN_SETUP_H

// List of Service and Characteristic UUIDs
#define SERVICE_UUID  "f9c521f6-0f14-4499-8f76-43116b40007d"
#define WIFI_UUID     "23456f8d-4aa7-4a61-956b-39c9bce0ff00"
#include "NimBLEDevice.h"
#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "ArduinoJson.h"
#include <Preferences.h>


class BluetoothCallbacks : public BLEServerCallbacks {
public:
    BluetoothCallbacks();

    void onConnect(BLEServer *pServer) override;

    void onDisconnect(BLEServer *pServer) override;
};

class BluetoothCallbackHandler : public BLECharacteristicCallbacks {
public:
    BluetoothCallbackHandler();

    void onWrite(BLECharacteristic *pCharacteristic) override;

    void onRead(BLECharacteristic *pCharacteristic) override;
};

void initBLE();

void gotIP();

void lostCon();

bool connectWiFi();

void setupWlan();

void createName();

void waitForBluetoothConnection();


#endif //ESP32_WLED_API_SERVER_WLAN_SETUP_H
