#ifndef ESP32_WLED_API_SERVER_BLUETOOTH_H
#define ESP32_WLED_API_SERVER_BLUETOOTH_H

// List of Service and Characteristic UUIDs
#define SERVICE_UUID  "f9c521f6-0f14-4499-8f76-43116b40007d"
#define WIFI_UUID     "23456f8d-4aa7-4a61-956b-39c9bce0ff00"

#include "NimBLEDevice.h"
#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "ArduinoJson.h"
#include <Preferences.h>

static BLECharacteristic *pCharacteristicWiFi{};
static BLEAdvertising *pBluetoothAdvertising{};
static BLEService *pBluetoothService{};
static BLEServer *pBluetoothServer{};
static bool dataReceived = false;
static String m_ssid;
static String m_password;
static String m_macAddress;

class Bluetooth {
public:

    static void bluetoothInit();

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
    static bool getDataReceived();

    static void stopAdvertising();

    static void createName();

    static void setDataReceived(bool received);

    static void setWifiCharacteristicValue(String value);

    static void notifyWifiCharacteristic();
};

#endif //ESP32_WLED_API_SERVER_BLUETOOTH_H
